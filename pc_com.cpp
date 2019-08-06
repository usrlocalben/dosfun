#include "pc_com.hpp"

#include "alg_ringindex.hpp"
#include "pc_pic.hpp"

#include <array>
#include <cstdio>
#include <stdexcept>
#include <string_view>

namespace rqdq {
namespace pc {

volatile int txCnt{0};

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

ComPort* theHack;

ComPort::ComPort(int ioBase, int irqNum, int bps, int flowType)
	:ioBase_(ioBase),
	irqLine_(irqNum),
	bps_(bps) {
		DLAB(0);  // force DLAB=0

		{ pc::CriticalSection section;
			IER(0);  // disable all UART interrupts
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

		MCR(MC_DTR|MC_RTS|MC_OUT1|MC_OUT2);

		int enabled = 0;
		enabled |= 0x01;  // rx data available
		// enabled |= 0x02;  // thr empty
		enabled |= 0x04;  // line status change
		enabled |= 0x08;  // modem status change
		IER(enabled);

		irqLine_.Connect(); }


ComPort::~ComPort() {
	Disable(); }


void ComPort::Disable() {
	pc::CriticalSection section;
	IER(0);
	MCR(0);
	FCR(0);
	for (int i=0; i<16; i++) RBR();
	IIR();

	LSR(); IIR();
	MSR(); IIR();
	LSR(); IIR();
	irqLine_.Disconnect();
	irqLine_.RestoreISR(); }


void ComPort::isrJmp() {
	theHack->isr(); }


inline void ComPort::isr() {
	irqLine_.Disconnect();
	pc::EnableInterrupts();

	bool done = false;
	while (1) {
		const int signal = IIR() & 7;
		if (signal == IIR_NO_INTERRUPT_PENDING) {
			break; }
		switch (signal) {
		case IIR_RX_DATA_AVAILABLE:
			isr_OnDataAvailable();
			break;
		case IIR_LINE_STATUS_CHANGE:
			isr_OnLineStatusChanged();
			break;
		case IIR_MODEM_STATUS_CHANGE:
			isr_OnModemStatusChanged();
			break;
		case IIR_THR_EMPTY:
			isr_OnTXBufferEmpty();
			break; }}

	{
		pc::CriticalSection section;
		irqLine_.SignalEOI();
		irqLine_.Connect(); }}


/**
 * drain RX bytes from the UART
 */
void ComPort::isr_OnDataAvailable() {
	while (LSR() & LS_DATA_READY) {
		char data = RBR();

		/* Handle XON/XOFF flow control (TX) */
		if (flowControlType_ == FLOW_XONXOFF && (data==XOFF || data==XON)) {
			auto willFlow = data == XON;
			if (!txFlowing_ && willFlow) {
				// transition from off to on
				if (txRing_.Loaded()) {
					IER(IER() | IE_THR_EMPTY); }}
			txFlowing_ = willFlow;
			return; }

		if (rxRing_.Full()) {
			return; }  // no room in buffer

		rxBuf_[rxRing_.BackIdx()] = data;
		rxBuf_[rxRing_.BackIdx()+kBufferSize] = data;
		rxRing_.PushBack();

		if (rxFlowing_ && RXBufferDanger()) {
			rxFlowing_ = false;
			switch (flowControlType_) {
			case FLOW_RTSCTS:
				MCR(MCR() & ~MC_RTS);
				break;
			case FLOW_NONE:
				break;
			case FLOW_XONXOFF:
				THR(XOFF);
				break;
			case FLOW_DTRDSR:
				MCR(MCR() & ~MC_DTR);
				break; }}}}


void ComPort::isr_OnLineStatusChanged() {
	lsr_ = LSR(); }


void ComPort::isr_OnModemStatusChanged() {
	msr_ = MSR();

	if (flowControlType_ == FLOW_RTSCTS) {
		txFlowing_ = (msr_ & MS_CTS) != 0; }
	else if (flowControlType_ == FLOW_DTRDSR) {
		txFlowing_ = (msr_ & MS_DSR) != 0; }
	
	if (txRing_.Loaded() && txFlowing_) {
		IER(IER() | IE_THR_EMPTY); }}


void ComPort::isr_OnTXBufferEmpty() {
	for (int cnt=0; cnt<16 && txFlowing_ && txRing_.Loaded(); cnt++) {
		THR(txBuf_[txRing_.FrontIdx()]); txRing_.PopFront(); }
	if (txRing_.Empty() || !txFlowing_) {
		IER(IER() & ~IE_THR_EMPTY); }}


void ComPort::SetBaudRate(int bps) {
	int divisor = BaudDivisor(bps);
	DIV(divisor); }


void ComPort::SetWordSize(int bits) {
	if (5 <= bits && bits <= 8) {
		LCR(LCR() & 0xfc | (bits-5)); }
	else {
		throw std::runtime_error("bad data size"); }}


void ComPort::SetParityMode(char mode) {
	if (mode != 'N') {
		throw std::runtime_error("expected parity N"); }
	// 11000111
	LCR(LCR() & 0xc7); }


void ComPort::SetStopSize(int bits) {
	if (bits != 1) {
		throw std::runtime_error("expected stop size 1"); }
	// 11111011
	LCR(LCR() & 0xfb); }


void ComPort::SetFlowControl(int fc) {
	txFlowing_ = true;
	rxFlowing_ = true;
	flowControlType_ = fc; }


void ComPort::SetThreshold(int level) {
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

	FCR(enable | rx_reset | tx_reset | level); }


bool ComPort::RXBufferDanger() const {
	return rxRing_.Size() > rxRing_.Capacity() * kRxBufDangerPct; }


bool ComPort::RXBufferSafe() const {
	return rxRing_.Size() < rxRing_.Capacity() * kRxBufSafePct; }



char ComPort::RBR() {
	return InB(ioBase_+RI_RBR); }
char ComPort::IER() {
	return InB(ioBase_+RI_IER); }
char ComPort::IIR() {
	return InB(ioBase_+RI_IIR); }
char ComPort::LCR() {
	return InB(ioBase_+RI_LCR); }
char ComPort::MCR() {
	return InB(ioBase_+RI_MCR); }
char ComPort::LSR() {
	return InB(ioBase_+RI_LSR); }
char ComPort::MSR() {
	return InB(ioBase_+RI_MSR); }
int ComPort::DIV() {
	DLAB(1);
	int value = InW(ioBase_+RI_DLL);
	DLAB(0);
	return value; }

void ComPort::THR(char value) {
	OutB(ioBase_+RI_THR, value); }
void ComPort::IER(char value) {
	OutB(ioBase_+RI_IER, value); }
void ComPort::FCR(char value) {
	OutB(ioBase_+RI_FCR, value); }
void ComPort::LCR(char value) {
	OutB(ioBase_+RI_LCR, value); }
void ComPort::MCR(char value) {
	OutB(ioBase_+RI_MCR, value); }
void ComPort::DIV(int value) {
	DLAB(1);
	OutW(ioBase_+RI_DLL, value);
	DLAB(0); }


void ComPort::DLAB(int num) {
	// DLAB bit is LCR bit 7.
	OutB(ioBase_+RI_LCR, (InB(ioBase_+RI_LCR)&0x7f)|(num<<7)); }


void ComPort::MaybeStartRXFlowing() {
	if (!rxFlowing_ && RXBufferSafe()) {
		rxFlowing_ = true;
		if (flowControlType_ == FLOW_XONXOFF) {
			THR(XON); }
		else if (flowControlType_ == FLOW_RTSCTS) {
			MCR(MCR()|MC_RTS); }
		else if (flowControlType_ == FLOW_DTRDSR) {
			MCR(MCR()|MC_DTR); }}}


std::string_view ComPort::Peek(int limit) const {
	int begin = rxRing_.FrontIdx();
	int size = rxRing_.Size();
	return { rxBuf_+begin, std::size_t(std::min(limit, size)) }; }


void ComPort::Ack(std::string_view seg) {
	rxRing_.PopFront(seg.size()); }


int ComPort::Write(std::string_view buf) {
	pc::CriticalSection section;
	int many = 0;
	for (const auto ch : buf) {
		if (txRing_.Full()) {
			break; }
		txBuf_[txRing_.BackIdx()] = ch;
		txRing_.PushBack();
		++many; }

	if (txRing_.Loaded()) {
		IER(IER() | IE_THR_EMPTY); }
	return many; }


/*
ComPort MakeComPort(std::string descr, int baud) {
	if (descr == "com1") {
		return ComPort(0x3f8, 4, baud); }
	else if (descr == "com2") {
		return ComPort(0x2f8, 3, baud); }
	else if (descr == "com3") {
		return ComPort(0x3e8, 4, baud); }
	else if (descr == "com4") {
		return ComPort(0x2e8, 3, baud); }
	throw std::runtime_error("bad serial descr"); }
*/


}  // namespace pc
}  // namespace rqdq
