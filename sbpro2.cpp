#include "sbpro2.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <conio.h>  // outp/inp

#include "pc_dma.hpp"
#include "pc_pic.hpp"

using std::uint8_t;
using std::uint16_t;
using std::int8_t;
using std::int16_t;

#define nullptr (0)

namespace rqdq {
namespace {

const int kSampleSizeInBytes = 1;


}  // namespace

namespace snd {

SBPro2Ports make_ports(int baseAddr) {
	SBPro2Ports out;
	out.reset = baseAddr + 0x06;
	out.read  = baseAddr + 0x0a;
	out.write = baseAddr + 0x0c;
	out.poll  = baseAddr + 0x0e;
	out.ack16 = baseAddr + 0x0f;
	return out; }


class SBPro2* theBlaster = 0;

uint8_t lo(uint16_t value) { return value & 0x00ff; }
uint8_t hi(uint16_t value) { return value >> 8; }


SBPro2::SBPro2(int baseAddr, int irqNum, int dmaChannelNum, int sampleRateInHz, int numChannels, int bufferSizeInSamples)
	:port_(make_ports(baseAddr)),
	irqLine_(irqNum),
	dma_(dmaChannelNum),
	sampleRateInHz_(sampleRateInHz),
	numChannels_(numChannels),
	bufferSizeInSamples_(bufferSizeInSamples),
	userBuffer_(1),
	playBuffer_(0),
	dmaBuffer_(bufferSizeInSamples_*numChannels_),  // words, *2 for buffers is built-in
	good_(false),
	userProc_(nullptr),
	userPtr_(nullptr)
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
	std::cout << "HW: " << std::hex << hwInfo << std::dec << "\n";
	*/

	_disable();
	irqLine_.Disconnect();
	irqLine_.SaveVect();
	irqLine_.SetVect(SBPro2::isrJmp);
	irqLine_.Connect();
	_enable();

	dmaBuffer_.Zero();
	dma_.Setup(dmaBuffer_);

	// set output sample rate
	TX(0x40);
	int tmp = 256 - (1000000 / (numChannels_ * sampleRateInHz_));
	TX(tmp);

	// enable speaker
	TX(0xd1);

	/*
	TX(0xc6);  // 8-bit, DAC, auto-init, fifo-enable
	TX(0x10);  // mode: mono, signed
	TX(lo(bufferSizeInSamples_*numChannels_-1));
	TX(hi(bufferSizeInSamples_*numChannels_-1));
	*/

	TX(0x48);
	TX(lo(bufferSizeInSamples_*numChannels_-1));
	TX(hi(bufferSizeInSamples_*numChannels_-1));

	TX(0x1c); }  // start auto-init playback


SBPro2::~SBPro2() {
	TX(0xd0);  // pause 8-bit DMA
	TX(0xd3);  // turn off speaker

	_disable();
	dma_.Stop();
	irqLine_.RestoreVect();
	_enable();

	RESET();
	SpinUntilReset(); }


inline void SBPro2::SpinUntilReadyForWrite() {
	while (inp(port_.write) & 0x80); }


inline void SBPro2::SpinUntilReadyForRead() {
	while (!(inp(port_.poll) & 0x80)); }


void SBPro2::TX(uint8_t value) {
	SpinUntilReadyForWrite();
	outp(port_.write, value); }


uint8_t SBPro2::RX() {
	SpinUntilReadyForRead();
	return inp(port_.read); }


void SBPro2::RESET() {
	outp(port_.reset, 1);
	outp(port_.reset, 0); }


bool SBPro2::SpinUntilReset() {
	int attempts = 100;
	while ((RX() != 0xaa) && attempts--);
	return attempts != 0; }


static void __interrupt SBPro2::isrJmp() {
	theBlaster->isr(); }


inline int8_t* SBPro2::GetUserBuffer() const {
	int8_t* dst = (int8_t*)dmaBuffer_.Ptr16();
	dst += userBuffer_*bufferSizeInSamples_*numChannels_;
	return dst; }


void SBPro2::isr() {
	// if (!IsRealIRQ(irqLine_)) { return; }
	irqLine_.SignalEOI();
	_enable();

	std::swap(userBuffer_, playBuffer_);
	int8_t* dst = GetUserBuffer();
	if (userProc_ != nullptr) {
		userProc_(dst, 1, numChannels_, bufferSizeInSamples_, userPtr_); }
	else {
		for (int i=0; i<bufferSizeInSamples_*numChannels_; i++) {
			dst[i] = 0; }}

	ACK(); }


inline void SBPro2::ACK() {
	inp(port_.poll); }


void SBPro2::AttachProc(audioproc userProc, void* userPtr) {
	_disable();
	userPtr_ = userPtr;
	userProc_ = userProc;
	_enable(); }


void SBPro2::DetachProc() {
	userProc_ = 0; }


bool SBPro2::IsGood() const {
	return good_; }


}  // namespace snd
}  // namespace rqdq
