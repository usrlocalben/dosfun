#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>

#include "app_kefrens_bars.hpp"
#include "app_player_adapter.hpp"
#include "kb_tinymod.hpp"
#include "ost.hpp"
#include "pc_kbd.hpp"
#include "pc_pit.hpp"
#include "sb16.hpp"
#include "vga_mode.hpp"
#include "vga_pageflip.hpp"
#include "vga_reg.hpp"
#include "vga_softvbi.hpp"

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


class Demo {
public:
	Demo()
		:quitSoon_(false),
		mCnt_(0),
		paulaPtr_(new kb::Paula()),
		playerPtr_(new kb::ModPlayer(paulaPtr_.get(), (uint8_t*)ostData)) {}

	void Run() {
		pc::Keyboard kbd;

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
		std::auto_ptr<PlayerAdapter> adapterPtr(new PlayerAdapter(*playerPtr_));
		blaster.AttachProc(PlayerAdapter::BlasterJmp, adapterPtr.get());

		quitSoon_ = false;
		while (!quitSoon_) {
			if (kbd.IsDataAvailable()) {
				pc::Event ke = kbd.GetMessage();
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
		pc::Stopwatch drawtime;
		DrawKefrensBars(vram, T, patternNum, rowNum);
		if (mCnt_ < kNumDrawTimeSamples) {
			float m = drawtime.GetElapsedTimeInSeconds();
			if (m > 0) {
				mLst_[mCnt_++] = m; }}
#ifdef SHOW_TIMING
		vga::SetRGB(0, 0,0,0);
#endif
		}

	void OnKeyDown(int scanCode) {
		if (scanCode == pc::SC_ESC) {
			quitSoon_ = true; }}

private:
	bool quitSoon_;
	std::auto_ptr<kb::Paula> paulaPtr_;
	std::auto_ptr<kb::ModPlayer> playerPtr_;

public:
	float measuredRefreshRateInHz_;
	float mLst_[kNumDrawTimeSamples];
	int mCnt_; };


}  // namespace app
}  // namespace rqdq


int main() {
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
