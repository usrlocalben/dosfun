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

constexpr int kResetSpinLimit{100};

// class snd::Blaster::impl* runningInstance = nullptr;
void* runningInstance = nullptr;

auto lo(uint16_t value) -> uint8_t { return value & 0x00ff; }
auto hi(uint16_t value) -> uint8_t { return value >> 8; }


class DSP {
	const int io_;

public:
	DSP(int io) : io_(io) {}

	auto ResetAddr() const -> int {
		return io_ + 0x06; }

	auto ReadAddr() const -> int {
		return io_ + 0x0a; }

	auto WriteAddr() const -> int {
		return io_ + 0x0c; }

	auto PollAddr() const -> int {
		return io_ + 0x0e; }

	template <int BITS>
	auto AckAddr() const -> int {
		if constexpr (BITS == 8) {
			return PollAddr(); }
		return io_ + 0x0f; }

	void SpinUntilReadyForWrite() const {
		while (InB(WriteAddr()) & 0x80); }

	void SpinUntilReadyForRead() const {
		while (!(InB(PollAddr()) & 0x80)); }

	void TX(uint8_t value) const {
		SpinUntilReadyForWrite();
		OutB(WriteAddr(), value); }

	auto RX() const -> uint8_t {
		SpinUntilReadyForRead();
		return InB(ReadAddr()); }

	/**
	 * section 2-2 "Resetting DSP"
	 * must wait 3usec before clearing reset bit
	 */
	void RESET() const {
		OutB(ResetAddr(), 1);
		for (int n=0; n<10; ++n) {
			InB(ReadAddr()); }
		OutB(ResetAddr(), 0); }

	auto SpinUntilResetOrTimeout() const -> bool {
		for (int n=0; n<50000; ++n) {
			if ((InB(PollAddr()) & 0x80) != 0) {
				uint8_t data = InB(ReadAddr());
				if (data == 0xaa) {
					return true; }}}
		return false; }

	template <int BITS>
	void ACK() const {
		InB(AckAddr<BITS>()); }};


/**
 * reset and retrieve hardware revision for SB DSP at port
 */
auto Detect(int io) -> uint16_t {
	DSP dsp(io);
	dsp.RESET();
	bool good = dsp.SpinUntilResetOrTimeout();
	if (!good) {
		log::info("sb16: no DSP detected at 0x%03x (timeout awaiting ready after reset)", io);
		return 0; }
	dsp.TX(0xe1);  // get hw version
	uint16_t hw;
	hw = dsp.RX() << 8;
	hw |= dsp.RX();
	log::info("sb16: found hardware version %04x", hw);
	if (0x200 < hw && hw < 0x400) {
		log::info("sb16: warning: sb2.01+ and sbpro support incomplete, using sb2.0 limits");
		hw = 0x200; }
	return hw; }


/**
 * given a valid version and _desired_ width,
 * returns the closest available width
 */
auto BestWidth(int v, int w) -> int {
	if (v < 0x300) {
		// sb1/2
		return 1; }
	return std::clamp(w, 1, 2); }


/**
 * compute bTimeConstant according to the SB docs
 *
 * the full 16-bit value is returned, _not_ the
 * 8-bit high-byte truncated value
 */
auto TC(int w, int r) -> int {
	auto tc = 65536 - (256'000'000 / (w*r));
	return tc; }


/**
 * compute the inverse of the time constant
 */
auto TCInverse(int w, int tc) -> int {
	int rate = 256'000'000 / ((65536-tc)*w);
	return rate; }


/**
 * given a valid version, width and _desired_ rate
 * returns the closest available valid rate
 *
 * for sb1/2/pro, the rate is computed based on the
 * transfer time constant
 */
auto BestRate(int v, int r, int w) -> int {
	if (v <= 0x200) {
		// sb1/2
		assert(w==1);
		r = std::clamp(r, 4000, 23000);
		r = TCInverse(1, TC(1, r)&0xff00);
		return r; }
	if (v < 0x300) {
		// sb 2.01+
		assert(w==1);
		r = std::clamp(r, 4000, 44100);
		r = TCInverse(1, TC(1, r)&0xff00);
		return r; }
	if (v < 0x400) {
		// sbpro
		if (w==1) {
			// mono
			r = std::clamp(r, 4000, 44100); }
		else {
			// stereo
			r = std::clamp(r, 11025, 22050); }
		r = TCInverse(1, TC(1, r)&0xff00);
		return r; }

	// sb16
	return std::clamp(r, 5000, 44100); }


/**
 * given a valid version/rate/width combination,
 * returns true indicating High-Speed DMA mode
 * is required
 */
auto RequiredMode(int v, int r, int w) -> bool {
	if (v == 0) {
		return false; }
	if (v <= 0x200) {
		// sb1/2
		assert(w==1);
		assert(4000 <= r && r <= 23000);
		return false; }
	if (v < 0x300) {
		// sb 2.01+
		assert(w==1);
		assert(4000 <= r && r <= 44100);
		return r > 23000; }
	if (v < 0x400) {
		// sbpro
		if (w==1) {
			// mono
			assert(4000 <= r && r <= 44100);
			return r > 23000; }
		else {
			// stereo
			assert(11025 <= r && r <= 22050);
			return true; }}
	// sb16
	assert(5000 <= r && r <= 44100);
	return false; }


auto BytesPerSampleX(int width, int precision) -> int {
	return width * (precision==16?2:1); }


}  // namespace

namespace snd {

int spuriousIRQCnt = 0;

class Blaster::impl {

	const DSP dsp_;
	const int hardwareVersion_;
	pc::IRQLineRT irqLine_;
	const int bits_;
	const pc::DMAChannel dma_;
	const int width_;
	const int sampleRateInHz_;
	const bool highSpeed_;
	const int bufferSizeInSamples_;
	const pc::DMABuffer dmaBuffer_;
	uint8_t* playBuffer_;
	uint8_t* userBuffer_;
	audioproc userProc_;
	void* userPtr_;

public:
	/**
	 * detect DSP version and select best available precision,
	 * width and rate.
	 *
	 * priority 1: prefers 16-bit over 8-bit
	 * priority 2: nearest to user requested width if available
	 * priority 3: nearest to user requested rate
	 */
	impl(BlasterConfig config, int reqRate, int reqWidth, int bufferSizeInSamples) :
		dsp_(config.io),
		hardwareVersion_(Detect(config.io)),
		irqLine_(config.irq),
		bits_((hardwareVersion_>=0x400 && config.dma16!=-1) ? 16 : 8),
		dma_(bits_==16 ? config.dma16 : config.dma8),
		width_(BestWidth(hardwareVersion_, reqWidth)),
		sampleRateInHz_(BestRate(hardwareVersion_, reqRate, width_)),
		highSpeed_(RequiredMode(hardwareVersion_, sampleRateInHz_, width_)),
		bufferSizeInSamples_(bufferSizeInSamples),
		dmaBuffer_(2 * bufferSizeInSamples_ * BytesPerSampleX(width_, bits_)),
		playBuffer_(dmaBuffer_.Ptr() + __djgpp_conventional_base),
		userBuffer_(playBuffer_ + bufferSizeInSamples_*BytesPerSampleX(width_, bits_)),
		userProc_(nullptr),
		userPtr_(nullptr)
		{
			if (hardwareVersion_>=0x400 && config.dma16==-1) {
				log::info("sb16: warning: hardware is 16-bit capable, but no 16-bit DMA channel configured"); }

			log::info("sb16: precision: %d bits (wanted 16)", bits_);
			log::info("sb16:     width: %d channels (wanted %d)", width_, reqWidth);
			log::info("sb16:      rate: %d hz (wanted %d)", sampleRateInHz_, reqRate);
			log::info("sb16: using dma channel: %d", dma_.ChannelNum() + (bits_==16 ? 4 : 0));
			log::info("sb16: using transfer mode: %s", highSpeed_ ? "High-Speed" : "Normal"); }

	~impl() {
		if (runningInstance == this) {
			Stop(); }}

	auto BytesPerSample() const -> int {
		return BytesPerSampleX(width_, bits_); }

	auto BufferSizeInBytes() const -> int {
		return bufferSizeInSamples_ * BytesPerSample(); }

	auto Rate() const -> int {
		return sampleRateInHz_; }

	void Start() {
		assert(runningInstance = nullptr);
		runningInstance = this;

		if (hardwareVersion_ == 0) {
			log::info("sb16: hardware not found");
			return; }

		{
			pc::CriticalSection cs;
			irqLine_.Disconnect();
			irqLine_.SaveISR();
			irqLine_.SetISR(bits_ == 8 ? isrJmp<8> : isrJmp<16>);
			irqLine_.Connect(); }

		dmaBuffer_.Zero();
		dma_.Setup(dmaBuffer_);

		// set output sample rate
		SetSampleRate();
		SpeakerOn();
		StartDMA();

		log::info("sb16: stream started"); }

	void Stop() {
		assert(runningInstance == this);

		if (hardwareVersion_ == 0) {
			return; }

		StopDMA(); 

		{
			pc::CriticalSection cs;
			dma_.Stop();
			irqLine_.Disconnect();
			irqLine_.RestoreISR();
			runningInstance = nullptr; }

		dsp_.RESET();
		dsp_.SpinUntilResetOrTimeout(); }

	void AttachProc(audioproc userProc, void* userPtr) {
		pc::CriticalSection cs;
		userPtr_ = userPtr;
		userProc_ = userProc; }

	void DetachProc() {
		userProc_ = nullptr; }

private:
	void SpeakerOn() {
		if (bits_ == 8) {
			dsp_.TX(0xd1); }}

	void SpeakerOff() {
		if (bits_ == 8) {
			dsp_.TX(0xd3); }}

	void SetSampleRate() {
		if (bits_ == 8) {
			dsp_.TX(0x40);
			int tmp = 256 - (1000000 / (width_ * sampleRateInHz_));
			dsp_.TX(tmp); }
		else {
			dsp_.TX(0x41);
			dsp_.TX(hi(sampleRateInHz_));
			dsp_.TX(lo(sampleRateInHz_)); }}

	void StartDMA() {
		if (bits_ == 8) {
			/*
			TX(0xc6);  // 8-bit, DAC, auto-init, fifo-enable
			TX(0x10);  // mode: mono, signed
			TX(lo(bufferSizeInSamples_*width_-1));
			TX(hi(bufferSizeInSamples_*width_-1));
			*/
			dsp_.TX(0x48);
			dsp_.TX(lo(BufferSizeInBytes() - 1));
			dsp_.TX(hi(BufferSizeInBytes() - 1));

			/*
			0x1c = auto-init dma, normal,     mono, 8-bit
			0x90 = auto-init dma, high-speed, mono, 8-bit
			*/

			dsp_.TX(highSpeed_ ? 0x90 : 0x1c); }  // start auto-init playback
		else {
			dsp_.TX(0xb6);  // 16-bit DAC, A/I, FIFO
			if (width_ == 2) {
				dsp_.TX(0x30); }  // DMA mode: 16-bit signed stereo
			else {
				dsp_.TX(0x10); }  // DMA mode: 16-bit signed mono
			dsp_.TX(lo(BufferSizeInBytes()/2 - 1));
			dsp_.TX(hi(BufferSizeInBytes()/2 - 1)); }}

	void StopDMA() {
		if (bits_ == 8) {
			if (highSpeed_) {
				dsp_.RESET(); }   // high-speed mode requires a reset to exit
			else {
				dsp_.TX(0xd0);    // pause 8-bit DMA
				dsp_.TX(0xd3); }} // turn off speaker
		else {
			dsp_.TX(0xd5); }}  // pause output

	/**
	 * dma buffer switching ISR
	 */
	template <int PRECISION>
	static
	void isrJmp() {
		static_cast<impl*>(runningInstance)->isr<PRECISION>(); }
	template <int PRECISION>
	void isr() {
#ifdef DETECT_SPURIOUS_IRQ
		if (!irqLine_.IsReal()) {
			spuriousIRQCnt++;
			return; }
#endif
		pc::EnableInterrupts();

		std::swap(playBuffer_, userBuffer_);
		if (userProc_ != nullptr) {
			userProc_(userBuffer_, PRECISION, width_, bufferSizeInSamples_, userPtr_); }

		pc::DisableInterrupts();
		dsp_.ACK<PRECISION>();
		irqLine_.SignalEOI(); }

	/**
	 * ack-only ISR for switching sbpro to stereo-mode
	 */
	static
	void isrJmpPro() {
		static_cast<impl*>(runningInstance)->isrPro(); }
	void isrPro() {
#ifdef DETECT_SPURIOUS_IRQ
		if (!irqLine_.IsReal()) {
			spuriousIRQCnt++;
			return; }
#endif
		dsp_.ACK<8>();
		irqLine_.SignalEOI(); }};


Blaster::Blaster(BlasterConfig config, int reqRate, int reqWidth, int bufferSizeInSamples) :
	impl_(std::make_unique<impl>(config, reqRate, reqWidth, bufferSizeInSamples)) {}

Blaster::~Blaster() = default;

void Blaster::Start() {
	impl_->Start(); }
void Blaster::Stop() {
	impl_->Stop(); }
void Blaster::AttachProc(audioproc userProc, void* userPtr) {
	impl_->AttachProc(userProc, userPtr); }
void Blaster::DetachProc() {
	impl_->DetachProc(); }
auto Blaster::Rate() const -> int {
	return impl_->Rate(); }


}  // namespace snd
}  // namespace rqdq
