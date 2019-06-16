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

namespace rqdq {

const int TRANSFER_SIZE_IN_SAMPLES = 2048;

const int SAMPLE_SIZE_IN_WORDS = 1;


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
	dmaMem_(AllocDMABuffer(TRANSFER_SIZE_IN_SAMPLES*SAMPLE_SIZE_IN_WORDS*2)),
	audioProcPtr_(0),
	irqCount_(0),
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

	ConfigureTransfer(dma_, dmaMem_);

	// set output sample rate
	TX(0x41);
	TX(hi(sampleRateInHz_));
	TX(lo(sampleRateInHz_));

	// 16-bit DAC, A/I, FIFO
	TX(0xb6);

	// DMA mode: 16-bit signed mono
	TX(0x10);
	TX(lo(TRANSFER_SIZE_IN_SAMPLES*SAMPLE_SIZE_IN_WORDS-1));
	TX(hi(TRANSFER_SIZE_IN_SAMPLES*SAMPLE_SIZE_IN_WORDS-1)); }


Blaster::~Blaster() {
	TX(0xd5);  // pause output

	_disable();
	StopDMA(dma_);
	_dos_setvect(pic_.isrNum, oldBlasterISRPtr);
	_enable();
	// std::cout << "irqCount: " << irqCount_ << "\n";

	RESET();
	SpinUntilReset();
	FreeDMABuffer(dmaMem_); }


void Blaster::SpinUntilReadyForWrite() {
	while (inp(port_.write) & 0x80); }


void Blaster::SpinUntilReadyForRead() {
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


void Blaster::isr() {
	// if (!IsRealIRQ(pic_)) { return; }
	irqCount_++;
	
	std::swap(userBuffer_, playBuffer_);
	int16_t* dst = (int16_t*)dmaMem_.Ptr16();
	dst += userBuffer_*TRANSFER_SIZE_IN_SAMPLES*SAMPLE_SIZE_IN_WORDS;
	if (audioProcPtr_ != 0) {
		audioProcPtr_(dst, TRANSFER_SIZE_IN_SAMPLES); }
	else {
		for (int i=0; i<TRANSFER_SIZE_IN_SAMPLES; i++) {
			dst[i] = 0; }}

	ACK();
	SignalEOI(pic_); }


inline void Blaster::ACK() {
	inp(port_.ack16); }  // XXX port name?


void Blaster::AttachProc(audioproc value) {
	audioProcPtr_ = value; }


void Blaster::DetachProc() {
	audioProcPtr_ = 0; }


bool Blaster::IsGood() const {
	return good_; }


}  // namespace rqdq
