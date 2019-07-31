#include "app_kefrens_bars.hpp"
#include "app_player_adapter.hpp"
#include "kb_tinymod.hpp"
#include "log.hpp"
#include "ost.hpp"
#include "pc_com.hpp"
#include "pc_kbd.hpp"
#include "pc_pit.hpp"
#include "sb16.hpp"
#include "sb_detect.hpp"
#include "text.hpp"
#include "vga_irq.hpp"
#include "vga_mode.hpp"
#include "vga_pageflip.hpp"
#include "vga_reg.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <memory>

#include <sys/nearptr.h>

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
		llp(0),
		mCnt_(0),
		paulaPtr_(new kb::Paula()),
		playerPtr_(new kb::ModPlayer(paulaPtr_.get(), ostData.data())) {}

	void Run() {
#ifdef TTYCON
		pc::ComPort tty(0x2f8, 3, 115200, pc::FLOW_NONE);
#endif
		pc::Keyboard kbd;

		vga::ModeSetter modeSetter;
		modeSetter.Set(vga::VM_MODEX);
		vga::RetraceIRQ<vga::FlipPages> flipPagesIRQ;
		measuredRefreshRateInHz_ = flipPagesIRQ.GetHz();
		log::info("measuredRefreshRate = %4.2f hz", measuredRefreshRateInHz_);

		effectPtr_.reset(new KefrensBars());

		snd::Blaster blaster(kSoundBlasterIOBaseAddr,
		                     kSoundBlasterIRQNum,
		                     kSoundBlasterDMAChannelNum,
		                     kAudioSampleRateInHz,
		                     kAudioWidthInChannels,
		                     kAudioBufferSizeInSamples);
		std::unique_ptr<PlayerAdapter> adapterPtr(new PlayerAdapter(*playerPtr_));
		adapterPtr->Refill();
		blaster.AttachProc(PlayerAdapter::BlasterJmp, adapterPtr.get());

		log::info("system ready.");


		quitSoon_ = false;
		std::vector<char> events;
		const char MSG_KBD_DATA_AVAILABLE = 1;
		const char MSG_VGA_CAN_WRITE = 2;
		const char MSG_TTY_DATA_AVAILABLE = 3;
		const char MSG_TTY_CAN_WRITE = 4;

		auto WaitForMultipleObjects = [&](const std::vector<char>& lst) -> int {
			while (1) {
				pc::CriticalSection section;
				for (int idx=0; idx<lst.size(); idx++) {
					const auto& evt = lst[idx];
					switch (evt) {
					case MSG_KBD_DATA_AVAILABLE:
						if (kbd.IsDataAvailable()) return idx; break;
					case MSG_VGA_CAN_WRITE:
						if (vga::backLocked) return idx; break;
#ifdef TTYCON
					case MSG_TTY_DATA_AVAILABLE:
						if (tty.DataAvailable()) return idx; break;
					case MSG_TTY_CAN_WRITE:
						if (tty.CanWrite()) return idx; break;
#endif
					default:
						throw std::runtime_error("invalid event"); }}
				pc::Sleep(); }};


		while (!quitSoon_) {
			events.clear();
			events.push_back(MSG_VGA_CAN_WRITE);
			events.push_back(MSG_KBD_DATA_AVAILABLE);
#ifdef TTYCON
			events.push_back(MSG_TTY_DATA_AVAILABLE);
			if (log::Loaded()) {
				events.push_back(MSG_TTY_CAN_WRITE); }
#endif

			int idx = WaitForMultipleObjects(events);
			const auto msg = events[idx];

			if (msg == MSG_KBD_DATA_AVAILABLE) {
				pc::Event ke = kbd.GetMessage();
				if (ke.down) {
					OnKeyDown(ke.scanCode); }}
#ifdef TTYCON
			else if (msg == MSG_TTY_CAN_WRITE) {
				static std::string line;
				line.assign(log::at(log::FrontIdx()));
				line += "\r\n";
				std::string_view segment{ line.c_str()+llp, line.size() - llp };
				int sent = tty.Write(segment);
				llp += sent;
				if (llp == line.size()) {
					llp = 0;
					log::PopFront(); }}
			else if (msg == MSG_TTY_DATA_AVAILABLE) {
				char tmp[2048];
				auto seg = tty.Peek(128);
				sprintf(tmp, "tty: received %s", text::JsonStringify(seg).data());
				tty.Ack(seg);
				log::info(tmp); }
#endif
			else if (msg == MSG_VGA_CAN_WRITE) {
				vga::AnimationPage animationPage;
				assert(animationPage.IsLocked());
				Draw(animationPage.Get());
				adapterPtr->Refill(); }}}


private:
	void Draw(const vga::VRAMPage& vram) {
		float T = vga::GetTime() / measuredRefreshRateInHz_;
		int patternNum = playerPtr_->GetCurrentPos();
		int rowNum = playerPtr_->GetCurrentRow();
#ifdef SHOW_TIMING
		vga::Color(255, { 0x30, 0x30, 0x30 });
#endif
		pc::Stopwatch drawtime;
		effectPtr_->Draw(vram, T, patternNum, rowNum);
		if (mCnt_ < kNumDrawTimeSamples) {
			float m = drawtime.GetElapsedTimeInSeconds();
			if (m > 0) {
				mLst_[mCnt_++] = m; }}
#ifdef SHOW_TIMING
		vga::Color(255, { 0, 0, 0 });
#endif
		}

	void OnKeyDown(int scanCode) {
		if (scanCode == pc::SC_ESC) {
			quitSoon_ = true; }}

private:
	bool quitSoon_;
	int llp;
	std::unique_ptr<kb::Paula> paulaPtr_;
	std::unique_ptr<kb::ModPlayer> playerPtr_;
	std::unique_ptr<KefrensBars> effectPtr_;

public:
	float measuredRefreshRateInHz_;
	float mLst_[kNumDrawTimeSamples];
	int mCnt_; };


}  // namespace app
}  // namespace rqdq


int main() {
	rqdq::log::Reserve();
	const std::uint16_t before = rqdq::pc::GetPICMasks();

	rqdq::hw::BlasterDetectResult bd = rqdq::hw::DetectBlaster();
	if (!bd.found) {
		std::printf("BLASTER not found\n");
		return 1; }

	rqdq::app::kSoundBlasterIRQNum = bd.value.irqNum;
	rqdq::app::kSoundBlasterIOBaseAddr = bd.value.ioAddr;
	rqdq::app::kSoundBlasterDMAChannelNum = bd.value.BestDMA();

	std::printf("Found BLASTER addr=0x%x irq=%d dma=%d\n",
	            rqdq::app::kSoundBlasterIOBaseAddr,
	            rqdq::app::kSoundBlasterIRQNum,
	            rqdq::app::kSoundBlasterDMAChannelNum);

	if (!__djgpp_nearptr_enable()) {
		std::printf("can't enable nearptr.\n");
		return 1; }

	rqdq::app::Demo().Run();
	return 0; }
