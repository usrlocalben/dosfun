#include "app_kefrens_bars.hpp"
#include "app_player_adapter.hpp"
#include "data_ost.hpp"
#include "kb_tinymod.hpp"
#include "log.hpp"
#include "pc_com.hpp"
#include "pc_kbd.hpp"
#include "pc_pit.hpp"
#include "sb16.hpp"
#include "sb_detect.hpp"
#include "text.hpp"
#include "vga_bios.hpp"
#include "vga_irq.hpp"
#include "vga_mode.hpp"
#include "vga_pageflip.hpp"
#include "vga_reg.hpp"
#include "ryg.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>

#include <sys/nearptr.h>

using std::uint8_t;
using std::int16_t;
using std::uint16_t;
using std::make_unique;

namespace rqdq {
namespace app {

constexpr int kAudioBufferSizeInSamples = 128;
constexpr int kAudioSampleRateInHz = 22050;
constexpr int kAudioWidthInChannels = 2;
int kSoundBlasterIOBaseAddr = 0x220;
int kSoundBlasterIRQNum = 7;         // 760eld == 5
int kSoundBlasterDMAChannelNum = 5;  // 760eld == 1

constexpr char MSG_KBD_DATA_AVAILABLE = 1;
constexpr char MSG_VGA_CAN_WRITE = 2;
constexpr char MSG_TTY_DATA_AVAILABLE = 3;
constexpr char MSG_TTY_CAN_WRITE = 4;


class Demo {

	bool quitSoon_{false};
	std::unique_ptr<kb::Paula> paula_;
	std::unique_ptr<kb::ModPlayer> player_;
	std::unique_ptr<PlayerAdapter> adapter_;
#ifdef TTYCON
	std::unique_ptr<pc::ComPort> tty_;
	int llp_{0};
#endif
	pc::Keyboard kbd_;
	vga::ModeSetter modeSetter_;
	std::unique_ptr<KefrensBars> effect_;
	std::optional<vga::RetraceIRQ<vga::FlipPages>> flipPagesIRQ_;
	std::unique_ptr<snd::Blaster> blaster_;

public:
	Demo():
		paula_(make_unique<kb::Paula>()),
		player_(make_unique<kb::ModPlayer>(paula_.get(), data::ost.data())),
		adapter_(make_unique<PlayerAdapter>(*player_)),
#ifdef TTYCON
		tty_(make_unique<pc::ComPort>(0x2f8, 3, 115200, pc::FLOW_NONE)),
#endif
		kbd_(),
		modeSetter_() {}

	void Run() {
		modeSetter_.Set(320, 240, false);
		vga::BIOSUtil::Border(255);
		flipPagesIRQ_.emplace();
		log::info("refreshRate = %4.2f hz (measured)", flipPagesIRQ_->GetHz());

		effect_ = make_unique<KefrensBars>();

		adapter_->Refill();
		blaster_ = make_unique<snd::Blaster>(kSoundBlasterIOBaseAddr, kSoundBlasterIRQNum, kSoundBlasterDMAChannelNum,
		                                     kAudioSampleRateInHz, kAudioWidthInChannels, kAudioBufferSizeInSamples);
		blaster_->AttachProc(PlayerAdapter::BlasterJmp, adapter_.get());
		blaster_->Start();

		log::info("system ready.");

		std::vector<char> events;
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
				auto ke = kbd_.Pop();
				if (ke.down) {
					OnKeyDown(ke.code); }}
#ifdef TTYCON
			else if (msg == MSG_TTY_CAN_WRITE) {
				static std::string line;
				line.assign(log::at(log::FrontIdx()));
				line += "\r\n";
				std::string_view segment{ line.c_str()+llp_, line.size() - llp_ };
				int sent = tty_->Write(segment);
				llp_ += sent;
				if (llp_ == line.size()) {
					llp_ = 0;
					log::PopFront(); }}
			else if (msg == MSG_TTY_DATA_AVAILABLE) {
				char tmp[2048];
				auto seg = tty_->Peek(128);
				sprintf(tmp, "tty: received %s", text::JsonStringify(seg).data());
				tty_->Ack(seg);
				log::info(tmp); }
#endif
			else if (msg == MSG_VGA_CAN_WRITE) {
				vga::DrawContext drawContext;
				assert(drawContext.IsLocked());
				Draw(drawContext); }}}

private:
	auto WaitForMultipleObjects(const std::vector<char>& lst) -> int {
		while (1) {
			pc::CriticalSection section;
			for (int idx=0; idx<lst.size(); idx++) {
				const auto& evt = lst[idx];
				switch (evt) {
				case MSG_KBD_DATA_AVAILABLE:
					if (kbd_.Loaded()) return idx; break;
				case MSG_VGA_CAN_WRITE:
					if (vga::backLocked) return idx; break;
#ifdef TTYCON
				case MSG_TTY_DATA_AVAILABLE:
					if (tty_->DataAvailable()) return idx; break;
				case MSG_TTY_CAN_WRITE:
					if (tty_->CanWrite()) return idx; break;
#endif
				default:
					throw std::runtime_error("invalid event"); }}
			pc::Sleep(); }}

private:
	void Draw(vga::DrawContext& dc) {
		float T = vga::GetTime() / flipPagesIRQ_->GetHz();
		int patternNum = player_->GetCurrentPos();
		int rowNum = player_->GetCurrentRow();
#ifdef SHOW_TIMING
		vga::Color(255, { 0xc0, 0xc0, 0xc0 });
#endif
		pc::Stopwatch drawtime;
		effect_->Draw(dc, T, patternNum, rowNum);
#ifdef SHOW_TIMING
		vga::Color(255, { 0, 0, 0 });
#endif
		adapter_->Refill(); }

	void OnKeyDown(int code) {
		if (code == pc::SC_ESC) {
			quitSoon_ = true; }
		else {
			log::info("key %d %02x", code, code); }}};


}  // namespace app
}  // namespace rqdq


int main() {
	ryg::Init();
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

	std::cerr << "===== BEGIN LOG DUMP =====\n";
	while (rqdq::log::Loaded()) {
		auto ll = rqdq::log::at(rqdq::log::FrontIdx());
		std::cerr << ll << "\n";
		rqdq::log::PopFront(); }
	std::cerr << "====== END LOG DUMP ======\n";

	return 0; }
