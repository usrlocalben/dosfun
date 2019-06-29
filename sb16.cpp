#include "sb16.hpp"

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
	dma_(dmaChannelNum),
	bits_(dmaChannelNum < 4 ? 8 : 16),
	sampleRateInHz_(sampleRateInHz),
	numChannels_(numChannels),
	bufferSizeInSamples_(bufferSizeInSamples),
	userBuffer_(1),
	playBuffer_(0),
	dmaBuffer_(bufferSizeInSamples_*numChannels_ * (bits_ == 16 ? 2 : 1)),
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
	dma_.Setup(dmaBuffer_);

	// set output sample rate
	SetSampleRate();
	SpeakerOn();
	StartDMA(); }


void Blaster::SpeakerOn() {
	if (bits_ == 8) {
		TX(0xd1); }}


void Blaster::SpeakerOff() {
	if (bits_ == 8) {
		TX(0xd3); }}


void Blaster::SetSampleRate() {
	if (bits_ == 8) {
		TX(0x40);
		int tmp = 256 - (1000000 / (numChannels_ * sampleRateInHz_));
		TX(tmp); }
	else {
		TX(0x41);
		TX(hi(sampleRateInHz_));
		TX(lo(sampleRateInHz_)); }}


void Blaster::StartDMA() {
	if (bits_ == 8) {
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
	else {
		TX(0xb6);  // 16-bit DAC, A/I, FIFO
		if (numChannels_ == 2) {
			TX(0x30); }  // DMA mode: 16-bit signed stereo
		else {
			TX(0x10); }  // DMA mode: 16-bit signed mono
		TX(lo(bufferSizeInSamples_*kSampleSizeInWords*numChannels_-1));
		TX(hi(bufferSizeInSamples_*kSampleSizeInWords*numChannels_-1)); }}


void Blaster::StopDMA() {
	if (bits_ == 8) {
		TX(0xd0);    // pause 8-bit DMA
		TX(0xd3); }  // turn off speaker
	else {
		TX(0xd5); }}  // pause output


Blaster::~Blaster() {
	StopDMA(); 

	_disable();
	dma_.Stop();
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


inline void* Blaster::GetUserBuffer() const {
	if (bits_ == 8) {
		int8_t* dst = (int8_t*)dmaBuffer_.Ptr16();
		dst += userBuffer_*bufferSizeInSamples_*numChannels_;
		return dst; }
	else {
		int16_t* dst = (int16_t*)dmaBuffer_.Ptr16();
		dst += userBuffer_*bufferSizeInSamples_*numChannels_;
		return dst; }}


void Blaster::isr() {
	// if (!IsRealIRQ(irqLine_)) { return; }
	ACK();
	irqLine_.SignalEOI();
	_enable();

	std::swap(userBuffer_, playBuffer_);
	void* dst = GetUserBuffer();
	int fmt = (bits_ == 8 ? 1 : 2);
	if (userProc_ != nullptr) {
		userProc_(dst, fmt, numChannels_, bufferSizeInSamples_, userPtr_); }}


inline void Blaster::ACK() {
	if (bits_ == 8) {
		inp(port_.poll); }
	else {
		inp(port_.ack16); }}


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
