#include "sb16.hpp"

#include "log.hpp"
#include "pc_bus.hpp"
#include "pc_cpu.hpp"
#include "pc_dma.hpp"
#include "pc_pic.hpp"

#include <algorithm>
#include <cstdint>

#include <sys/nearptr.h>


using std::uint8_t;
using std::uint16_t;
using std::int8_t;
using std::int16_t;
using rqdq::pc::InB;
using rqdq::pc::OutB;

namespace rqdq {
namespace {

const int kSampleSizeInWords = 1;

// class snd::Blaster::impl* runningInstance = nullptr;
void* runningInstance = nullptr;

struct Ports {
	int reset;
	int read;
	int write;
	int poll;
	int ack16; };

Ports make_ports(int baseAddr) {
	Ports out;
	out.reset = baseAddr + 0x06;
	out.read  = baseAddr + 0x0a;
	out.write = baseAddr + 0x0c;
	out.poll  = baseAddr + 0x0e;
	out.ack16 = baseAddr + 0x0f;
	return out; }

uint8_t lo(uint16_t value) { return value & 0x00ff; }
uint8_t hi(uint16_t value) { return value >> 8; }

}  // namespace

namespace snd {

int spuriousIRQCnt = 0;

class Blaster::impl {
public:
	impl(int baseAddr, int irqNum, int dmaChannelNum, int sampleRateInHz, int numChannels, int bufferSizeInSamples) :
		port_(make_ports(baseAddr)),
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
		userPtr_(nullptr) {}

	void Start() {
		assert(runningInstance = nullptr);
		runningInstance = this;

		RESET();
		good_ = SpinUntilReset();
		if (!good_) {
			log::info("sb16: hardware not found");
			return; }

		TX(0xe1);  // get hw version info
		uint16_t hwInfo;
		hwInfo = RX() << 8;
		hwInfo |= RX();
		log::info("sb16: found hardware, version %04x", hwInfo);

		{
			pc::CriticalSection cs;
			irqLine_.Disconnect();
			irqLine_.SaveISR();
			irqLine_.SetISR(isrJmp);
			irqLine_.Connect(); }

		dmaBuffer_.Zero();
		dma_.Setup(dmaBuffer_);

		// set output sample rate
		SetSampleRate();
		SpeakerOn();
		StartDMA();

		log::info("sb16: stream started"); }

private:
	void SpeakerOn() {
		if (bits_ == 8) {
			TX(0xd1); }}

	void SpeakerOff() {
		if (bits_ == 8) {
			TX(0xd3); }}

	void SetSampleRate() {
		if (bits_ == 8) {
			TX(0x40);
			int tmp = 256 - (1000000 / (numChannels_ * sampleRateInHz_));
			TX(tmp); }
		else {
			TX(0x41);
			TX(hi(sampleRateInHz_));
			TX(lo(sampleRateInHz_)); }}

	void StartDMA() {
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

	void StopDMA() {
		if (bits_ == 8) {
			TX(0xd0);    // pause 8-bit DMA
			TX(0xd3); }  // turn off speaker
		else {
			TX(0xd5); }}  // pause output

public:
	void Stop() {
		assert(runningInstance == this);
		StopDMA(); 

		{
			pc::CriticalSection cs;
			dma_.Stop();
			irqLine_.Disconnect();
			irqLine_.RestoreISR();
			runningInstance = nullptr; }

		RESET();
		SpinUntilReset(); }

private:
	inline void SpinUntilReadyForWrite() {
		while (InB(port_.write) & 0x80); }

	inline void SpinUntilReadyForRead() {
		while (!(InB(port_.poll) & 0x80)); }

	void TX(uint8_t value) {
		SpinUntilReadyForWrite();
		OutB(port_.write, value); }

	uint8_t RX() {
		SpinUntilReadyForRead();
		return InB(port_.read); }

	void RESET() {
		OutB(port_.reset, 1);
		OutB(port_.reset, 0); }

	bool SpinUntilReset() {
		int attempts = 100;
		while ((RX() != 0xaa) && attempts--);
		return attempts != 0; }

	inline void* GetUserBuffer() const {
		if (bits_ == 8) {
			int8_t* dst = (int8_t*)dmaBuffer_.Ptr16();
			dst += userBuffer_*bufferSizeInSamples_*numChannels_;
			return dst; }
		else {
			int16_t* dst = (int16_t*)dmaBuffer_.Ptr16();
			dst += userBuffer_*bufferSizeInSamples_*numChannels_;
			return dst; }}

	static void isrJmp() {
		static_cast<impl*>(runningInstance)->isr(); }

	inline void isr() {
#ifdef DETECT_SURIOUS_IRQ
		if (!irqLine_.IsReal()) {
			spuriousIRQCnt++;
			return; }
#endif
		ACK();
		pc::EnableInterrupts();

		std::swap(userBuffer_, playBuffer_);
		void* dst = (char*)GetUserBuffer() + __djgpp_conventional_base;
		int fmt = (bits_ == 8 ? 1 : 2);
		if (userProc_ != nullptr) {
			userProc_(dst, fmt, numChannels_, bufferSizeInSamples_, userPtr_); }

		irqLine_.SignalEOI(); }

	inline void ACK() {
		if (bits_ == 8) {
			InB(port_.poll); }
		else {
			InB(port_.ack16); }}

public:
	void AttachProc(audioproc userProc, void* userPtr) {
		pc::CriticalSection cs;
		userPtr_ = userPtr;
		userProc_ = userProc; }

	void DetachProc() {
		userProc_ = nullptr; }

	bool IsGood() const {
		return good_; }

	~impl() {
		if (runningInstance == this) {
			Stop(); }}
private:
	const Ports port_;
	pc::IRQLineRT irqLine_;
	const pc::DMAChannel dma_;
	const int bits_;
	const int sampleRateInHz_;
	const int numChannels_;
	const int bufferSizeInSamples_;
	int userBuffer_;
	int playBuffer_;
	const pc::DMABuffer dmaBuffer_;
	bool good_;
	audioproc userProc_;
	void* userPtr_; };


Blaster::Blaster(int baseAddr, int irqNum, int dmaChannelNum, int sampleRateInHz, int numChannels, int bufferSizeInSamples) :
	impl_(std::make_unique<impl>(baseAddr, irqNum, dmaChannelNum, sampleRateInHz, numChannels, bufferSizeInSamples)) {}
Blaster::~Blaster() = default;

void Blaster::Start() {
	impl_->Start(); }
void Blaster::Stop() {
	impl_->Stop(); }
void Blaster::AttachProc(audioproc userProc, void* userPtr) {
	impl_->AttachProc(userProc, userPtr); }
void Blaster::DetachProc() {
	impl_->DetachProc(); }


}  // namespace snd
}  // namespace rqdq
