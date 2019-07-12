#include "pc_com.hpp"

#include <array>
#include <cstdio>
#include <stdexcept>

#include "pc_pic.hpp"

namespace rqdq {
namespace pc {

const float kRxBufDangerPct = 0.80f;
const float kRxBufSafePct = 0.20f;

int BaudDivisor(int baud) {
	const std::array<std::pair<int, int>, 5> table = { {
		{ 9600, 12 },
		{ 19200, 6 },
		{ 38400, 3 },
		{ 57600, 2 },
		{ 115200, 1 } } };

	for (const auto& item : table) {
		if (item.first == baud) {
			return item.second; }}
	throw std::runtime_error("invalid baud rate"); }

const int RI_RBR = 0;  // r
const int RI_THR = 0;  // w
const int RI_DLL = 0;  // rw
const int RI_DLM = 1;  // rw
const int RI_IER = 1;  // rw
const int RI_IIR = 2;  // r
const int RI_FCR = 2;  // w
const int RI_LCR = 3;  // rw
const int RI_MCR = 4;  // rw
const int RI_LSR = 5;  // r
const int RI_MSR = 6;  // r
const int RI_SCR = 7;  // rw

const int IIR_NO_INTERRUPT_PENDING = 1;  // 000:1
const int IIR_MODEM_STATUS_CHANGE = 0;   // 000:0
const int IIR_THR_EMPTY = 2;             // 010:0
const int IIR_RX_DATA_AVAILABLE = 4;     // 100:0
const int IIR_LINE_STATUS_CHANGE = 6;    // 110:0

const int IE_MODEM_STATUS = 0x08;
const int IE_ERRORS = 0x04;
const int IE_THR_EMPTY = 0x02;
const int IE_RX_DATA_READY = 0x01;

const int MC_DTR = 0x01;
const int MC_RTS = 0x02;
const int MC_OUT1 = 0x04;
const int MC_OUT2 = 0x08;

const int MS_CTS = 0x10;
const int MS_DSR = 0x20;

const int LS_DATA_READY = 0x01;
const int LS_THR_EMPTY = 0x20;
// const int LS_OVERRUN_ERROR = 2;

const char XON = 0x11;
const char XOFF = 0x13;

SerialStream* theHack;

SerialStream::SerialStream(int ioBase, int irqNum, int bps, int flowType)
	:ioBase_(ioBase),
	irqLine_(irqNum),
	bps_(bps) {
		SetDLAB(0);  // force DLAB=0

		{ pc::CriticalSection section;
			OutIER(0);  // disable all UART interrupts
			irqLine_.SaveISR();
			theHack = this;
			irqLine_.SetISR(isrJmp);
			irqLine_.Disconnect(); }

		SetBaudRate(bps);
		SetWordSize(8);
		SetParityMode('N');
		SetStopSize(1);
		SetFlowControl(flowType);
		SetThreshold(8);

		OutMCR(MC_DTR|MC_RTS|MC_OUT1|MC_OUT2);

		int enabled = 0;
		enabled |= 0x01;  // rx data available
		// enabled |= 0x02;  // thr empty
		enabled |= 0x04;  // line status change
		enabled |= 0x08;  // modem status change
		OutIER(enabled);

		irqLine_.Connect(); }


SerialStream::~SerialStream() {
	Disable(); }


void SerialStream::Disable() {
	pc::CriticalSection section;
	OutIER(0);
	OutMCR(0);
	OutFCR(0);
	for (int i=0; i<16; i++) InRBR();
	InIIR();

	InLSR(); InIIR();
	InMSR(); InIIR();
	InLSR(); InIIR();
	irqLine_.Disconnect();
	irqLine_.RestoreISR(); }


void SerialStream::isrJmp() {
	theHack->isr(); }


inline void SerialStream::isr() {
	irqLine_.Disconnect();
	pc::EnableInterrupts();
	irqLine_.SignalEOI();

	bool done = false;
	while (1) {
		const int signal = InIIR() & 7;
		if (signal == IIR_NO_INTERRUPT_PENDING) {
			break; }
		switch (signal) {
		case IIR_RX_DATA_AVAILABLE:
			OnDataReadyISR();
			break;
		case IIR_LINE_STATUS_CHANGE:
			OnLineStatusChange();
			break;
		case IIR_MODEM_STATUS_CHANGE:
			OnModemStatusChange();
			break;
		case IIR_THR_EMPTY:
			OnTXBufferEmpty();
			break; }}

	{
		pc::CriticalSection section;
		irqLine_.Connect(); }}


/**
 * drain RX bytes from the UART
 */
void SerialStream::OnDataReadyISR() {
	while (InLSR() & LS_DATA_READY) {
		char data = InRBR();

		/* Handle XON/XOFF flow control (TX) */
		if (flowControlType_ == FLOW_XONXOFF && (data==XOFF || data==XON)) {
			auto willFlow = data == XON;
			if (!txFlowing_ && willFlow) {
				// transition from off to on
				if (txRing_.Loaded()) {
					OutIER(InIER() | IE_THR_EMPTY); }}
			txFlowing_ = willFlow;
			return; }


		if (rxRing_.Full()) {
			return; }  // no room in buffer

		rxBuf_[rxRing_.BackIdx()] = data;
		rxRing_.PushBack();

		if (rxFlowing_ && RXBufferDanger()) {
			rxFlowing_ = false;
			switch (flowControlType_) {
			case FLOW_RTSCTS:
				OutMCR(InMCR() & ~MC_RTS);
				break;
			case FLOW_NONE:
				break;
			case FLOW_XONXOFF:
				OutTHR(XOFF);
				break;
			case FLOW_DTRDSR:
				OutMCR(InMCR() & ~MC_DTR);
				break; }}}}


void SerialStream::OnLineStatusChange() {
	lsr_ = InLSR(); }


void SerialStream::OnModemStatusChange() {
	msr_ = InMSR();

	if (flowControlType_ == FLOW_RTSCTS) {
		txFlowing_ = (msr_ & MS_CTS) != 0; }
	else if (flowControlType_ == FLOW_DTRDSR) {
		txFlowing_ = (msr_ & MS_DSR) != 0; }
	
	if (txRing_.Loaded() && txFlowing_) {
		OutIER(InIER() | IE_THR_EMPTY); }}


void SerialStream::OnTXBufferEmpty() {
	while (txFlowing_ && InLSR()&LS_THR_EMPTY && txRing_.Loaded()) {
		OutTHR(txBuf_[txRing_.FrontIdx()]); txRing_.PopFront(); }
	if (txRing_.Empty() || !txFlowing_) {
		OutIER(InIER() & ~IE_THR_EMPTY); }}


void SerialStream::SetBaudRate(int bps) {
	int divisor = BaudDivisor(bps);
	OutDIV(divisor); }

void SerialStream::SetWordSize(int bits) {
	if (5 <= bits && bits <= 8) {
		OutLCR(InLCR() & 0xfc | (bits-5)); }
	else {
		throw std::runtime_error("bad data size"); }}

void SerialStream::SetParityMode(char mode) {
	if (mode != 'N') {
		throw std::runtime_error("expected parity N"); }
	// 11000111
	OutLCR(InLCR() & 0xc7); }

void SerialStream::SetStopSize(int bits) {
	if (bits != 1) {
		throw std::runtime_error("expected stop size 1"); }
	// 11111011
	OutLCR(InLCR() & 0xfb); }

void SerialStream::SetFlowControl(int fc) {
	txFlowing_ = true;
	rxFlowing_ = true;
	flowControlType_ = fc; }

void SerialStream::SetThreshold(int level) {
	int enable = 0x01;
	int rx_reset = 0x02;
	int tx_reset = 0x04;
	if (level == 14) {
		level = 0xc0; }
	else if (level == 8) {
		level = 0x80; }
	else if (level == 4) {
		level = 0x40; }
	else if (level == 1) {
		level = 0x00; }
	else {
		level = 0x00; }

	OutFCR(enable | rx_reset | tx_reset | level); }


bool SerialStream::RXBufferDanger() const {
	return rxRing_.Size() > rxRing_.Capacity() * kRxBufDangerPct; }

bool SerialStream::RXBufferSafe() const {
	return rxRing_.Size() < rxRing_.Capacity() * kRxBufSafePct; }



char SerialStream::InRBR() {
	return InB(ioBase_+RI_RBR); }
char SerialStream::InIER() {
	return InB(ioBase_+RI_IER); }
char SerialStream::InIIR() {
	return InB(ioBase_+RI_IIR); }
char SerialStream::InLCR() {
	return InB(ioBase_+RI_LCR); }
char SerialStream::InMCR() {
	return InB(ioBase_+RI_MCR); }
char SerialStream::InLSR() {
	return InB(ioBase_+RI_LSR); }
char SerialStream::InMSR() {
	return InB(ioBase_+RI_MSR); }
int SerialStream::InDIV() {
	SetDLAB(1);
	int value = InW(ioBase_+RI_DLL);
	SetDLAB(0);
	return value; }

void SerialStream::OutTHR(char value) {
	OutB(ioBase_+RI_THR, value); }
void SerialStream::OutIER(char value) {
	OutB(ioBase_+RI_IER, value); }
void SerialStream::OutFCR(char value) {
	OutB(ioBase_+RI_FCR, value); }
void SerialStream::OutLCR(char value) {
	OutB(ioBase_+RI_LCR, value); }
void SerialStream::OutMCR(char value) {
	OutB(ioBase_+RI_MCR, value); }
void SerialStream::OutDIV(int value) {
	SetDLAB(1);
	OutW(ioBase_+RI_DLL, value);
	SetDLAB(0); }


void SerialStream::SetDLAB(int num) {
	// DLAB bit is LCR bit 7.
	OutB(ioBase_+RI_LCR, (InB(ioBase_+RI_LCR)&0x7f)|(num<<7)); }


void SerialStream::MaybeStartRXFlowing() {
	if (!rxFlowing_ && RXBufferSafe()) {
		rxFlowing_ = true;
		if (flowControlType_ == FLOW_XONXOFF) {
			OutTHR(XON); }
		else if (flowControlType_ == FLOW_RTSCTS) {
			OutMCR(InMCR()|MC_RTS); }
		else if (flowControlType_ == FLOW_DTRDSR) {
			OutMCR(InMCR()|MC_DTR); }}}


bool SerialStream::CanRead() const {
	return rxRing_.Loaded(); }


bool SerialStream::CanWrite() const {
	return txFlowing_ && !txRing_.Full(); }


int SerialStream::Read(char *buf, int len) {
	pc::CriticalSection section;
	int idx;
	for (idx = 0; idx < len && rxRing_.Loaded(); idx++) {
		buf[idx] = rxBuf_[rxRing_.FrontIdx()];  rxRing_.PopFront(); }
	MaybeStartRXFlowing();
	return idx; }


int SerialStream::Write(const char *buf, int len) {
	pc::CriticalSection section;
	int idx;
	for (idx = 0; idx < len; idx++) {
		if (txRing_.Full()) {
			break; }
		txBuf_[txRing_.BackIdx()] = buf[idx];
		txRing_.PushBack(); }

	if (txRing_.Loaded()) {
		OutIER(InIER() | IE_THR_EMPTY); }
	return idx; }


/*
SerialStream MakeSerialStream(std::string descr, int baud) {
	if (descr == "com1") {
		return SerialStream(0x3f8, 4, baud); }
	else if (descr == "com2") {
		return SerialStream(0x2f8, 3, baud); }
	else if (descr == "com3") {
		return SerialStream(0x3e8, 4, baud); }
	else if (descr == "com4") {
		return SerialStream(0x2e8, 3, baud); }
	throw std::runtime_error("bad serial descr"); }
*/


}  // namespace pc
}  // namespace rqdq
