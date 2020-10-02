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

constexpr int kSampleSizeInWords = 1;

// class snd::Blaster::impl* runningInstance = nullptr;
void* runningInstance = nullptr;

struct Ports {
	const int reset;
	const int read;
	const int write;
	const int poll;
	const int ack16; };

auto make_ports(int base) -> Ports {
	return { base+0x06, base+0x0a, base+0x0c, base+0x0e, base+0x0f }; }

auto lo(uint16_t value) -> uint8_t { return value & 0x00ff; }
auto hi(uint16_t value) -> uint8_t { return value >> 8; }

}  // namespace

namespace snd {

int spuriousIRQCnt = 0;

class Blaster::impl {

	pc::IRQLineRT irqLine_;
	const pc::DMAChannel dma_;
	const int bits_;
	const std::int16_t resetPort_;
	const std::int16_t readPort_;
	const std::int16_t writePort_;
	const std::int16_t pollPort_;
	const std::int16_t ackPort_;
	const int sampleRateInHz_;
	const int numChannels_;
	const int bufferSizeInSamples_;
	int playBuffer_;
	const pc::DMABuffer dmaBuffer_;
	bool good_;
	audioproc userProc_;
	void* userPtr_;

public:
	impl(int baseAddr, int irqNum, int dmaChannelNum, int sampleRateInHz, int numChannels, int bufferSizeInSamples) :
		irqLine_(irqNum),
		dma_(dmaChannelNum),
		bits_(dmaChannelNum < 4 ? 8 : 16),
		resetPort_(baseAddr + 0x06),
		readPort_(baseAddr + 0x0a),
		writePort_(baseAddr + 0x0c),
		pollPort_(baseAddr + 0x0e),
		ackPort_(bits_ == 8 ? pollPort_ : baseAddr + 0x0f),
		sampleRateInHz_(sampleRateInHz),
		numChannels_(numChannels),
		bufferSizeInSamples_(bufferSizeInSamples),
		playBuffer_(0),
		dmaBuffer_(bufferSizeInSamples_*numChannels_ * (bits_ == 16 ? 2 : 1)),
		good_(false),
		userProc_(nullptr),
		userPtr_(nullptr)
		{}

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
	void SpinUntilReadyForWrite() {
		while (InB(writePort_) & 0x80); }

	void SpinUntilReadyForRead() {
		while (!(InB(pollPort_) & 0x80)); }

	void TX(uint8_t value) {
		SpinUntilReadyForWrite();
		OutB(writePort_, value); }

	auto RX() -> uint8_t {
		SpinUntilReadyForRead();
		return InB(readPort_); }

	void RESET() {
		OutB(resetPort_, 1);
		OutB(resetPort_, 0); }

	auto SpinUntilReset() -> bool {
		int attempts = 100;
		while ((RX() != 0xaa) && attempts--);
		return attempts != 0; }

	auto GetUserBuffer() const -> void* {
		int userBuffer_ = playBuffer_ ^ 1;
		if (bits_ == 8) {
			int8_t* dst = (int8_t*)dmaBuffer_.Ptr16();
			dst += userBuffer_*bufferSizeInSamples_*numChannels_;
			return dst; }
		else {
			int16_t* dst = (int16_t*)dmaBuffer_.Ptr16();
			dst += userBuffer_*bufferSizeInSamples_*numChannels_;
			return dst; }}

	static
	void isrJmp() {
		static_cast<impl*>(runningInstance)->isr(); }

	void isr() {
#ifdef DETECT_SURIOUS_IRQ
		if (!irqLine_.IsReal()) {
			spuriousIRQCnt++;
			return; }
#endif
		pc::EnableInterrupts();

		playBuffer_ ^= 1;
		void* dst = (char*)GetUserBuffer() + __djgpp_conventional_base;
		int fmt = (bits_ == 8 ? 1 : 2);
		if (userProc_ != nullptr) {
			userProc_(dst, fmt, numChannels_, bufferSizeInSamples_, userPtr_); }

		pc::DisableInterrupts();
		ACK();
		irqLine_.SignalEOI(); }

	void ACK() {
		InB(ackPort_); }

public:
	void AttachProc(audioproc userProc, void* userPtr) {
		pc::CriticalSection cs;
		userPtr_ = userPtr;
		userProc_ = userProc; }

	void DetachProc() {
		userProc_ = nullptr; }

	auto IsGood() const -> bool {
		return good_; }

	~impl() {
		if (runningInstance == this) {
			Stop(); }}};


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
