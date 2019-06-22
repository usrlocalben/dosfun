#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>

#include "efx.hpp"
#include "fli.hpp"
#include "kbd.hpp"
#include "mod.hpp"
#include "ost.hpp"
#include "snd.hpp"
#include "vga.hpp"
#include "pit.hpp"

using std::uint8_t;
using std::int16_t;
using std::uint16_t;

namespace rqdq {
namespace app {

const int kAudioBufferSizeInSamples = 128;
const int kAudioSampleRateInHz = 22050;
const int kAudioWidthInChannels = 2;
const int kSoundBlasterIOBaseAddr = 0x220;
const int kSoundBlasterIRQNum = 0x7;
const int kSoundBlasterDMAChannelNum = 0x05;

const int kNumDrawTimeSamples = 500;


class PlayerAdapter {
public:
	PlayerAdapter(mod::Player& p) :player_(p) {}

	static void BlasterJmp(int16_t* out, int numChannels, int numSamples, void* self) {
		static_cast<PlayerAdapter*>(self)->BlasterProc(out, numChannels, numSamples); }
private:
	void BlasterProc(int16_t* out, int numChannels, int numSamples) {
#ifdef SHOW_TIMING
vga::SetRGB(0, 0x20, 0x3f, 0x10);
#endif
		player_.Render(pbuf_, pbuf_+4096, numSamples);
#ifdef SHOW_TIMING
vga::SetRGB(0, 0, 0, 0);
#endif
		for (int i=0; i<numSamples; i++) {
			if (numChannels == 2) {
				out[i*2+0] = pbuf_[i]      * std::numeric_limits<int16_t>::max();
				out[i*2+1] = pbuf_[i+4096] * std::numeric_limits<int16_t>::max(); }
			else {
				out[i] = ((pbuf_[i]+pbuf_[i+4096])*0.5f) * std::numeric_limits<int16_t>::max(); }}}

private:
	float pbuf_[4096*2];
	mod::Player& player_; };


class Demo {
public:
	Demo()
		:quitSoon_(false),
		mCnt_(0),
		paulaPtr_(new mod::Paula()),
		playerPtr_(new mod::Player(paulaPtr_.get(), (uint8_t*)ostData)) {}

	void Run() {
		kbd::Keyboard kbd;

		vga::ModeSetter modeSetter;
		modeSetter.Set(vga::VM_MODEX);
		vga::SoftVBI softVBI(&vga::vbi);
		measuredRefreshRateInHz_ = softVBI.GetFrequency();

		snd::Blaster blaster(kSoundBlasterIOBaseAddr,
							 kSoundBlasterIRQNum,
							 kSoundBlasterDMAChannelNum,
							 kAudioSampleRateInHz,
							 kAudioWidthInChannels,
							 kAudioBufferSizeInSamples);
		std::shared_ptr<PlayerAdapter> adapterPtr(new PlayerAdapter(*playerPtr_));
		blaster.AttachProc(PlayerAdapter::BlasterJmp, adapterPtr.get());

		quitSoon_ = false;
		while (!quitSoon_) {
			if (kbd.IsDataAvailable()) {
				kbd::Event ke = kbd.GetMessage();
				if (ke.down) {
					OnKeyDown(ke.scanCode); }
				continue; }

			vga::VRAMLock vramLock;
			if (vramLock.IsLocked()) {
				Draw(vramLock.Page()); }}}

private:
	void Draw(const vga::VRAMPage& vram) {
		float T = vga::GetTime() / measuredRefreshRateInHz_;
		int patternNum = playerPtr_->GetCurrentPos();
		int rowNum = playerPtr_->GetCurrentRow();
#ifdef SHOW_TIMING
		vga::SetRGB(0, 0x30, 0x30, 0x30);
#endif
		pit::Stopwatch drawtime;
		efx::DrawKefrensBars(vram, T, patternNum, rowNum);
		if (mCnt_ < kNumDrawTimeSamples) {
			float m = drawtime.GetElapsedTimeInSeconds();
			if (m > 0) {
				mLst_[mCnt_++] = m; }}
#ifdef SHOW_TIMING
		vga::SetRGB(0, 0,0,0);
#endif
		}

	void OnKeyDown(int scanCode) {
		if (scanCode == kbd::SC_ESC) {
			quitSoon_ = true; }}

private:
	bool quitSoon_;
	std::shared_ptr<mod::Paula> paulaPtr_;
	std::shared_ptr<mod::Player> playerPtr_;

public:
	float measuredRefreshRateInHz_;
	float mLst_[kNumDrawTimeSamples];
	int mCnt_; };


}  // namespace app
}  // namespace rqdq


int main(int argc, char *argv[]) {
	rqdq::app::Demo demo;
	demo.Run();

	float ax = 0;
	for (int i=0; i<demo.mCnt_; i++) {
		ax += demo.mLst_[i]; }
	ax /= demo.mCnt_;

	std::cout << "        elapsedTime: " << rqdq::vga::GetTime() << " frames\n";
	std::cout << "measuredRefreshRate:   " << demo.measuredRefreshRateInHz_ << " hz\n";
	std::cout << "        avgDrawTime:   " << (ax*1000) << " ms\n";
	return 0; }
