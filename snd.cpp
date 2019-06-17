#include "snd.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <conio.h>  // outp/inp
#include <dos.h>  // _dos_setvect

#include "dma.hpp"
#include "pic.hpp"

using std::uint8_t;
using std::uint16_t;
using std::int16_t;

#define nullptr (0)

namespace {

const int kBufferSizeInSamples = 512;
const int kSampleSizeInWords = 1;


}  // namespace

namespace rqdq {

Ports make_ports(int baseAddr) {
	Ports out;
	out.reset = baseAddr + 0x06;
	out.read  = baseAddr + 0x0a;
	out.write = baseAddr + 0x0c;
	out.poll  = baseAddr + 0x0e;
	out.ack16 = baseAddr + 0x0f;
	return out; }


void (__interrupt * oldBlasterISRPtr)();

class Blaster* theBlaster = 0;

uint8_t lo(uint16_t value) { return value & 0x00ff; }
uint8_t hi(uint16_t value) { return value >> 8; }


Blaster::Blaster(int ioAddr, int irqNum, int dmaNum, int rate)
	:port_(make_ports(ioAddr)),
	pic_(make_picinfo(irqNum)),
	dma_(make_dmainfo(dmaNum)),
	sampleRateInHz_(rate),
	userBuffer_(1),
	playBuffer_(0),
	dmaMem_(AllocDMABuffer(kBufferSizeInSamples*kSampleSizeInWords*2)),
	audioProcPtr_(0),
	good_(false)
{
	theBlaster = this;

	RESET();
	good_ = SpinUntilReset();
	if (!good_) {
		return; }

	/*
	TX(0xe1);  // get hw version info
	uint16_t hwInfo;
	hwInfo = RX() << 8;
	hwInfo |= RX();
	*/

	_disable();
	outp(pic_.maskPort, (inp(pic_.maskPort)|pic_.stopMask));
	oldBlasterISRPtr = _dos_getvect(pic_.isrNum);
	_dos_setvect(pic_.isrNum, &Blaster::isrJmp);
	outp(pic_.maskPort, (inp(pic_.maskPort)&pic_.startMask));
	_enable();

	std::memset(dmaMem_.Ptr(), 0, dmaMem_.sizeInWords*2);

	ConfigureTransfer(dma_, dmaMem_);

	// set output sample rate
	TX(0x41);
	TX(hi(sampleRateInHz_));
	TX(lo(sampleRateInHz_));

	// 16-bit DAC, A/I, FIFO
	TX(0xb6);

	// DMA mode: 16-bit signed mono
	TX(0x10);
	TX(lo(kBufferSizeInSamples*kSampleSizeInWords-1));
	TX(hi(kBufferSizeInSamples*kSampleSizeInWords-1)); }


Blaster::~Blaster() {
	TX(0xd5);  // pause output

	_disable();
	StopDMA(dma_);
	_dos_setvect(pic_.isrNum, oldBlasterISRPtr);
	_enable();

	RESET();
	SpinUntilReset();
	FreeDMABuffer(dmaMem_); }


inline void Blaster::SpinUntilReadyForWrite() {
	while (inp(port_.write) & 0x80); }


inline void Blaster::SpinUntilReadyForRead() {
	while (!(inp(port_.poll) & 0x80)); }


void Blaster::TX(uint8_t value) {
	SpinUntilReadyForWrite();
	outp(port_.write, value); }


uint8_t Blaster::RX() {
	SpinUntilReadyForRead();
	return inp(port_.read); }


void Blaster::RESET() {
	outp(port_.reset, 1);
	outp(port_.reset, 0); }


bool Blaster::SpinUntilReset() {
	int attempts = 100;
	while ((RX() != 0xaa) && attempts--);
	return attempts != 0; }


static void __interrupt Blaster::isrJmp() {
	theBlaster->isr(); }


inline int16_t* Blaster::GetUserBuffer() const {
	int16_t* dst = (int16_t*)dmaMem_.Ptr16();
	dst += userBuffer_*kBufferSizeInSamples*kSampleSizeInWords;
	return dst; }


void Blaster::isr() {
	// if (!IsRealIRQ(pic_)) { return; }
	SignalEOI(pic_);
	_enable();

	std::swap(userBuffer_, playBuffer_);
	int16_t* dst = GetUserBuffer();
	if (audioProcPtr_ != nullptr) {
		audioProcPtr_(dst, kBufferSizeInSamples); }
	else {
		for (int i=0; i<kBufferSizeInSamples; i++) {
			dst[i] = 0; }}

	ACK(); }
	//SignalEOI(pic_); }


inline void Blaster::ACK() {
	inp(port_.ack16); }  // XXX port name?


void Blaster::AttachProc(audioproc value) {
	audioProcPtr_ = value; }


void Blaster::DetachProc() {
	audioProcPtr_ = 0; }


bool Blaster::IsGood() const {
	return good_; }


}  // namespace rqdq
