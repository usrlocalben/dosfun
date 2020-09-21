#include "vga_irq.hpp"

#include "log.hpp"
#include "pc_pit.hpp"

#include <cstdint>

namespace rqdq {
namespace {

constexpr int kNumVBISamples = 30;


}  // close unnamed namespace
namespace vga {

int irqSleepTimeInTicks = 0;


auto CalibrateFrameTimer() -> int {
	std::uint16_t ax{0xffff};
	for (int n=0; n<kNumVBISamples; ++n) {
		SpinUntilNextRetraceBegins();
		pc::BeginMeasuring();
		SpinUntilNextRetraceBegins();
		auto t = pc::EndMeasuring();
		log::info("vga: calibrating %d/%d: %d", n, kNumVBISamples, t);
		ax = std::min(ax, t); }
	log::info("vga: calibrated refresh, period = %d ticks", ax);
	return ax; }


}  // namespace vga
}  // namespace rqdq
