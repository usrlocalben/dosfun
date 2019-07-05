#include "sb16.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>

#include "pc_bus.hpp"
#include "pc_cpu.hpp"
#include "pc_dma.hpp"
#include "pc_pic.hpp"

using std::uint8_t;
using std::uint16_t;
using std::int8_t;
using std::int16_t;
using rqdq::pc::InB;
using rqdq::pc::OutB;

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


class Blaster* theBlaster = nullptr;

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

	{
		pc::CriticalSection cs;
		irqLine_.Disconnect();
		irqLine_.SaveISR();
		irqLine_.SetISR(Blaster::isrJmp);
		irqLine_.Connect(); }

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

	{
		pc::CriticalSection cs;
		dma_.Stop();
		irqLine_.RestoreISR(); }

	RESET();
	SpinUntilReset(); }


inline void Blaster::SpinUntilReadyForWrite() {
	while (InB(port_.write) & 0x80); }


inline void Blaster::SpinUntilReadyForRead() {
	while (!(InB(port_.poll) & 0x80)); }


void Blaster::TX(uint8_t value) {
	SpinUntilReadyForWrite();
	OutB(port_.write, value); }


uint8_t Blaster::RX() {
	SpinUntilReadyForRead();
	return InB(port_.read); }


void Blaster::RESET() {
	OutB(port_.reset, 1);
	OutB(port_.reset, 0); }


bool Blaster::SpinUntilReset() {
	int attempts = 100;
	while ((RX() != 0xaa) && attempts--);
	return attempts != 0; }


inline void* Blaster::GetUserBuffer() const {
	if (bits_ == 8) {
		int8_t* dst = (int8_t*)dmaBuffer_.Ptr16();
		dst += userBuffer_*bufferSizeInSamples_*numChannels_;
		return dst; }
	else {
		int16_t* dst = (int16_t*)dmaBuffer_.Ptr16();
		dst += userBuffer_*bufferSizeInSamples_*numChannels_;
		return dst; }}


void __interrupt Blaster::isrJmp() {
	theBlaster->isr(); }


inline void Blaster::isr() {
	// if (!IsRealIRQ(irqLine_)) { return; }
	ACK();
	irqLine_.SignalEOI();
	pc::EnableInterrupts();

	std::swap(userBuffer_, playBuffer_);
	void* dst = GetUserBuffer();
	int fmt = (bits_ == 8 ? 1 : 2);
	if (userProc_ != nullptr) {
		userProc_(dst, fmt, numChannels_, bufferSizeInSamples_, userPtr_); }}


inline void Blaster::ACK() {
	if (bits_ == 8) {
		InB(port_.poll); }
	else {
		InB(port_.ack16); }}


void Blaster::AttachProc(audioproc userProc, void* userPtr) {
	pc::CriticalSection cs;
	userPtr_ = userPtr;
	userProc_ = userProc; }


void Blaster::DetachProc() {
	userProc_ = nullptr; }


bool Blaster::IsGood() const {
	return good_; }


}  // namespace snd
}  // namespace rqdq
