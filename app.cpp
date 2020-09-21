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

class Demo {

	std::unique_ptr<kb::Paula> paula_;
	std::unique_ptr<kb::ModPlayer> player_;
	std::unique_ptr<PlayerAdapter> adapter_;
	vga::ModeSetter modeSetter_;
	std::unique_ptr<snd::Blaster> blaster_;

public:
	Demo():
		paula_(make_unique<kb::Paula>()),
		player_(make_unique<kb::ModPlayer>(paula_.get(), data::ost.data())),
		adapter_(make_unique<PlayerAdapter>(*player_)),
		modeSetter_() {}

	void Run() {

		modeSetter_.Set(vga::VM_MODEX);

		const int kPasses = 4;
		{pc::BeginMeasuring();
		uint8_t* dst = vga::VRAM_ADDR + __djgpp_conventional_base;
		for (int pass=0; pass<kPasses; ++pass) {
			vga::Planes(1<<(pass%4));
			for (int i=0; i<16384; ++i) {
				dst[i] = static_cast<uint8_t>(pass); }}
		int dt = pc::EndMeasuring();
		pc::StartSquareWave(0);
		int totalBytes = kPasses * 16384;
		float totalSeconds = pc::ticksToSeconds(dt);
		float rate = totalBytes/(1024.0F*1024)/totalSeconds;
		// log::info("elapsed time: %d ticks, %.4f secs", dt, totalSeconds);
		// log::info("bytes written: %d", totalBytes);
		log::info("8-bit writes to 0xa0000 %.4f MiB/sec", rate);}

		{pc::BeginMeasuring();
		uint16_t* dst = reinterpret_cast<uint16_t*>(vga::VRAM_ADDR + __djgpp_conventional_base);
		for (int pass=0; pass<kPasses; ++pass) {
			vga::Planes(1<<(pass%4));
			for (int i=0; i<16384/2; ++i) {
				dst[i] = static_cast<uint16_t>(pass); }}
		int dt = pc::EndMeasuring();
		pc::StartSquareWave(0);
		int totalBytes = kPasses * 16384;
		float totalSeconds = pc::ticksToSeconds(dt);
		float rate = totalBytes/(1024.0F*1024)/totalSeconds;
		// log::info("elapsed time: %d ticks, %.4f secs", dt, totalSeconds);
		// log::info("bytes written: %d", totalBytes);
		log::info("16-bit writes to 0xa0000 %.4f MiB/sec", rate);}

		{pc::BeginMeasuring();
		uint32_t* dst = reinterpret_cast<uint32_t*>(vga::VRAM_ADDR + __djgpp_conventional_base);
		for (int pass=0; pass<kPasses; ++pass) {
			vga::Planes(1<<(pass%4));
			for (int i=0; i<16384/4; ++i) {
				dst[i] = static_cast<uint32_t>(pass); }}
		int dt = pc::EndMeasuring();
		pc::StartSquareWave(0);
		int totalBytes = kPasses * 16384;
		float totalSeconds = pc::ticksToSeconds(dt);
		float rate = totalBytes/(1024.0F*1024)/totalSeconds;
		// log::info("elapsed time: %d ticks, %.4f secs", dt, totalSeconds);
		// log::info("bytes written: %d", totalBytes);
		log::info("32-bit writes to 0xa0000 %.4f MiB/sec", rate);}

		/*
		adapter_->Refill();
		blaster_ = make_unique<snd::Blaster>(kSoundBlasterIOBaseAddr, kSoundBlasterIRQNum, kSoundBlasterDMAChannelNum,
		                                     kAudioSampleRateInHz, kAudioWidthInChannels, kAudioBufferSizeInSamples);
		blaster_->AttachProc(PlayerAdapter::BlasterJmp, adapter_.get());
		blaster_->Start();
		*/
		{pc::BeginMeasuring();
		uint8_t* dst = vga::VRAM_ADDR + __djgpp_conventional_base;
		for (int pass=0; pass<kPasses; ++pass) {
			vga::Planes(1<<(pass%4));
			for (int i=0; i<16384; ++i) {
				dst[i] = static_cast<uint8_t>(pass); }}
		int dt = pc::EndMeasuring();
		pc::StartSquareWave(0);
		int totalBytes = kPasses * 16384;
		float totalSeconds = pc::ticksToSeconds(dt);
		float rate = totalBytes/(1024.0F*1024)/totalSeconds;
		// log::info("elapsed time: %d ticks, %.4f secs", dt, totalSeconds);
		// log::info("bytes written: %d", totalBytes);
		log::info("8-bit writes to 0xa0000 %.4f MiB/sec", rate);}

		{pc::BeginMeasuring();
		uint16_t* dst = reinterpret_cast<uint16_t*>(vga::VRAM_ADDR + __djgpp_conventional_base);
		for (int pass=0; pass<kPasses; ++pass) {
			vga::Planes(1<<(pass%4));
			for (int i=0; i<16384/2; ++i) {
				dst[i] = static_cast<uint16_t>(pass); }}
		int dt = pc::EndMeasuring();
		pc::StartSquareWave(0);
		int totalBytes = kPasses * 16384;
		float totalSeconds = pc::ticksToSeconds(dt);
		float rate = totalBytes/(1024.0F*1024)/totalSeconds;
		// log::info("elapsed time: %d ticks, %.4f secs", dt, totalSeconds);
		// log::info("bytes written: %d", totalBytes);
		log::info("16-bit writes to 0xa0000 %.4f MiB/sec", rate);}

		{pc::BeginMeasuring();
		uint32_t* dst = reinterpret_cast<uint32_t*>(vga::VRAM_ADDR + __djgpp_conventional_base);
		for (int pass=0; pass<kPasses; ++pass) {
			vga::Planes(1<<(pass%4));
			for (int i=0; i<16384/4; ++i) {
				dst[i] = static_cast<uint32_t>(pass); }}
		int dt = pc::EndMeasuring();
		pc::StartSquareWave(0);
		int totalBytes = kPasses * 16384;
		float totalSeconds = pc::ticksToSeconds(dt);
		float rate = totalBytes/(1024.0F*1024)/totalSeconds;
		// log::info("elapsed time: %d ticks, %.4f secs", dt, totalSeconds);
		// log::info("bytes written: %d", totalBytes);
		log::info("32-bit writes to 0xa0000 %.4f MiB/sec", rate);}

		return; }};


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
