#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <memory>
#include <sys/nearptr.h>

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
		std::unique_ptr<PlayerAdapter> adapterPtr(new PlayerAdapter(*playerPtr_));
		adapterPtr->Refill();
		blaster.AttachProc(PlayerAdapter::BlasterJmp, adapterPtr.get());

		quitSoon_ = false;
		char msgs[16];
		int msgCnt;
		const int MSG_KBD_DATA_AVAILABLE = 1;
		const int MSG_VGA_PAGE_LOCKED = 2;
		const int MSG_SND_BUFFER_LOW = 3;
		while (!quitSoon_) {
			{
				pc::CriticalSection section;
				msgCnt = 0;
				if (kbd.IsDataAvailable()) {
					msgs[msgCnt++] = MSG_KBD_DATA_AVAILABLE; }
				if (vga::backLocked) {
					msgs[msgCnt++] = MSG_VGA_PAGE_LOCKED; }
				if (!adapterPtr->Full()) {
					msgs[msgCnt++] = MSG_SND_BUFFER_LOW; }
				if (msgCnt == 0) {
					pc::Sleep();
					continue; }}

			for (int mi=0; mi<msgCnt; mi++) {
				char& msg = msgs[mi];
				if (msg == MSG_KBD_DATA_AVAILABLE) {
					pc::Event ke = kbd.GetMessage();
					if (ke.down) {
						OnKeyDown(ke.scanCode); }}
				else if (msg == MSG_VGA_PAGE_LOCKED) {
					vga::AnimationPage animationPage;
					if (animationPage.IsLocked()) {
						Draw(animationPage.Get()); }}
				else if (msg == MSG_SND_BUFFER_LOW) {
					adapterPtr->Refill(); }}}}

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
	std::unique_ptr<kb::Paula> paulaPtr_;
	std::unique_ptr<kb::ModPlayer> playerPtr_;

public:
	float measuredRefreshRateInHz_;
	float mLst_[kNumDrawTimeSamples];
	int mCnt_; };


}  // namespace app
}  // namespace rqdq


int main() {
	rqdq::hw::BlasterDetectResult bd = rqdq::hw::DetectBlaster();
	if (!bd.found) {
		std::printf("BLASTER not found\n");
		return 1; }

	rqdq::app::kSoundBlasterIOBaseAddr = bd.value.ioAddr;
	rqdq::app::kSoundBlasterIRQNum = bd.value.irqNum;
	rqdq::app::kSoundBlasterDMAChannelNum = bd.value.BestDMA();

	std::printf("Found BLASTER addr=0x%x irq=%d dma=%d\n",
	            rqdq::app::kSoundBlasterIOBaseAddr,
	            rqdq::app::kSoundBlasterIRQNum,
	            rqdq::app::kSoundBlasterDMAChannelNum);

	if (!__djgpp_nearptr_enable()) {
		std::printf("can't enable nearptr.\n");
		return 1; }

	rqdq::app::Demo demo;
	demo.Run();

	float ax = 0;
	for (int i=0; i<demo.mCnt_; i++) {
		ax += demo.mLst_[i]; }
	ax /= demo.mCnt_;

	std::printf("measuredRefreshRate: %.2f hz\n", demo.measuredRefreshRateInHz_);
	std::printf("        avgDrawTime: %.2f ms\n", (ax*1000));
	// std::printf("        spuriousIRQ: %d occurrences\n", rqdq::snd::spuriousIRQCnt);
	return 0; }
