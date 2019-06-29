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
#include "sb_detect.hpp"
#include "vga_mode.hpp"
#include "vga_pageflip.hpp"
#include "vga_reg.hpp"
#include "vga_irq.hpp"

using std::uint8_t;
using std::int16_t;
using std::uint16_t;

#define nullptr (0)

namespace rqdq {
namespace app {

const int kAudioBufferSizeInSamples = 128;
const int kAudioSampleRateInHz = 22050;
const int kAudioWidthInChannels = 1;
int kSoundBlasterIOBaseAddr = 0x220;
int kSoundBlasterIRQNum = 7;         // 760eld == 5
int kSoundBlasterDMAChannelNum = 5;  // 760eld == 1

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
		vga::RetraceIRQ<vga::FlipPages> flipPagesIRQ;
		measuredRefreshRateInHz_ = flipPagesIRQ.GetHz();

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

			vga::AnimationPage animationPage;
			if (animationPage.IsLocked()) {
				Draw(animationPage.Get()); }}}

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
	rqdq::hw::BlasterDetectResult bd = rqdq::hw::DetectBlaster();
	if (!bd.found) {
		std::cout << "BLASTER not found\n";
		std::exit(1); }

	rqdq::app::kSoundBlasterIOBaseAddr = bd.value.ioAddr;
	rqdq::app::kSoundBlasterIRQNum = bd.value.irqNum;
	rqdq::app::kSoundBlasterDMAChannelNum = bd.value.BestDMA();

	std::cout << "Found BLASTER";
	std::cout << " addr=0x" << std::hex << rqdq::app::kSoundBlasterIOBaseAddr << std::dec;
	std::cout << " irq=" << rqdq::app::kSoundBlasterIRQNum;
	std::cout << " dma=" << rqdq::app::kSoundBlasterDMAChannelNum;
	std::cout << "\n";
	// std::exit(0);

	rqdq::app::Demo demo;
	demo.Run();

	float ax = 0;
	for (int i=0; i<demo.mCnt_; i++) {
		ax += demo.mLst_[i]; }
	ax /= demo.mCnt_;

	std::cout << "        elapsedTime: " << std::dec << rqdq::vga::GetTime() << " frames\n";
	std::cout << "measuredRefreshRate:   " << demo.measuredRefreshRateInHz_ << " hz\n";
	std::cout << "        avgDrawTime:   " << (ax*1000) << " ms\n";
	return 0; }
