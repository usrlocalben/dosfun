#include "snd.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <conio.h>  // outp/inp

#include "dma.hpp"
#include "pic.hpp"

using std::uint8_t;
using std::uint16_t;
using std::int16_t;

#define nullptr (0)

namespace rqdq {
namespace {

const int kSampleSizeInWords = 1;


}  // namespace

namespace snd {

Ports make_ports(int baseAddr) {
	Ports out;
	out.reset = baseAddr + 0x06;
	out.read  = baseAddr + 0x0a;
	out.write = baseAddr + 0x0c;
	out.poll  = baseAddr + 0x0e;
	out.ack16 = baseAddr + 0x0f;
	return out; }


class Blaster* theBlaster = 0;

uint8_t lo(uint16_t value) { return value & 0x00ff; }
uint8_t hi(uint16_t value) { return value >> 8; }


Blaster::Blaster(int baseAddr, int irqNum, int dmaChannelNum, int sampleRateInHz, int numChannels, int bufferSizeInSamples)
	:port_(make_ports(baseAddr)),
	irqLine_(irqNum),
	dma_(dma::make_channel(dmaChannelNum)),
	sampleRateInHz_(sampleRateInHz),
	numChannels_(numChannels),
	bufferSizeInSamples_(bufferSizeInSamples),
	userBuffer_(1),
	playBuffer_(0),
	dmaBuffer_(bufferSizeInSamples_*numChannels_*kSampleSizeInWords*2),
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
	*/

	_disable();
	irqLine_.Disconnect();
	irqLine_.SaveVect();
	irqLine_.SetVect(Blaster::isrJmp);
	irqLine_.Connect();
	_enable();

	dmaBuffer_.Zero();
	dma::Configure(dma_, dmaBuffer_);

	// set output sample rate
	TX(0x41);
	TX(hi(sampleRateInHz_));
	TX(lo(sampleRateInHz_));

	TX(0xb6);  // 16-bit DAC, A/I, FIFO
	if (numChannels_ == 2) {
		TX(0x30); }  // DMA mode: 16-bit signed stereo
	else {
		TX(0x10); }  // DMA mode: 16-bit signed mono
	TX(lo(bufferSizeInSamples_*kSampleSizeInWords*numChannels_-1));
	TX(hi(bufferSizeInSamples_*kSampleSizeInWords*numChannels_-1)); }


Blaster::~Blaster() {
	TX(0xd5);  // pause output

	_disable();
	dma::Stop(dma_);
	irqLine_.RestoreVect();
	_enable();

	RESET();
	SpinUntilReset(); }


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
	int16_t* dst = (int16_t*)dmaBuffer_.Ptr16();
	dst += userBuffer_*bufferSizeInSamples_*numChannels_*kSampleSizeInWords;
	return dst; }


void Blaster::isr() {
	// if (!IsRealIRQ(irqLine_)) { return; }
	irqLine_.SignalEOI();
	_enable();

	std::swap(userBuffer_, playBuffer_);
	int16_t* dst = GetUserBuffer();
	if (userProc_ != nullptr) {
		userProc_(dst, numChannels_, bufferSizeInSamples_, userPtr_); }
	else {
		for (int i=0; i<bufferSizeInSamples_*numChannels_; i++) {
			dst[i] = 0; }}

	ACK(); }


inline void Blaster::ACK() {
	inp(port_.ack16); }


void Blaster::AttachProc(audioproc userProc, void* userPtr) {
	_disable();
	userPtr_ = userPtr;
	userProc_ = userProc;
	_enable(); }


void Blaster::DetachProc() {
	userProc_ = 0; }


bool Blaster::IsGood() const {
	return good_; }


}  // namespace snd
}  // namespace rqdq
