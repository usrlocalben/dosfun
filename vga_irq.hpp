#pragma once
#include "log.hpp"
#include "pc_cpu.hpp"
#include "pc_pic.hpp"
#include "pc_pit.hpp"
#include "vga_reg.hpp"

#include <cstdlib>
#include <cstdint>

namespace rqdq {
namespace vga {

// typedef void (*vbifunc)();

/**
 * Frame/PIT jitter buffer, as a percentage of frame-time.
 *
 * The smaller this value is, the less time will be
 * consumed spinning before retrace begins.
 *
 * However, if this value is _too small_, timing jitter may
 * cause the ISR to be called _after_ retrace has already
 * started.
 *
 * This would be disastrous: the effective software VBI
 * period would be longer than the VGA frame.  The IRQ-raise
 * time would drift slightly with each successive frame,
 * eventually being raised _after_ retrace causing the cpu
 * to spin for an entire display period.  Additionally, the
 * wall-clock timer provided by the IRQ would miss a tick.
 */
constexpr int kJitterFactor = (99<<16)/100;  // 0.99 16:16 fixed-point

extern int irqSleepTimeInTicks;

auto CalibrateFrameTimer() -> int;

template <typename VBIPROC>
class RetraceIRQ {

	int frameDurationInTicks_;

public:
	/**
	 * Install the IRQ system.
	 *
	 * Calibrates the timer by measuring the refresh-to-refresh
	 * time for a VGA frame using the PIT.  Then, the ISR is
	 * installed and triggered on the following frame.
	 */
	RetraceIRQ() :
		frameDurationInTicks_(CalibrateFrameTimer()) {

		irqSleepTimeInTicks =
			(frameDurationInTicks_ * kJitterFactor) >> 16;

		SpinWhileRetracing();
		pc::pitIRQLine.SaveISR();
		pc::pitIRQLine.SetISR(RetraceIRQ::OnBeforeRetrace);
		{
			pc::CriticalSection cs;
			SpinUntilRetracing();
			pc::StartCountdown(irqSleepTimeInTicks); }

		log::info("vga: RetraceIRQ installed"); }

	// non-copyable
	auto operator=(const RetraceIRQ&) -> RetraceIRQ& = delete;
	RetraceIRQ(const RetraceIRQ&) = delete;

	/**
	 * Restore the BIOS timer ISR and interval
	 */
	~RetraceIRQ() {
		pc::pitIRQLine.RestoreISR();
		pc::StartSquareWave(0);
		log::info("vga: RetraceIRQ uninstalled, PIT restored"); }

public:
	auto GetHz() const -> float {
		return pc::ticksToHz(frameDurationInTicks_); }

	/**
	 * Timer-based VBI ISR
	 *
	 * Execution should begin _just before_ VGA blanking/retrace
	 * begins.
	 *
	 * The CPU will spin until the rising-edge of the retrace
	 * signal is detected, then immediately begin the next PIT
	 * countdown before calling the user's VBI function.
	 *
	 * See also kJitterPct
	 */
	static
	void OnBeforeRetrace() {
		/*
		 * when execution begins, retrace still hasn't started
		 */
#ifdef SHOW_TIMING
		Color(255, {0xff,0x7f,0xff});
#endif
		vga::SpinUntilRetracing();

		/*
		 * retrace just started.
		 * reset the timer, then call the user's vbi handler
		 */
		pc::StartCountdown(irqSleepTimeInTicks);
#ifdef SHOW_TIMING
		Color(255, {0,0,0});
#endif
		VBIPROC()();
		pc::pitIRQLine.SignalEOI(); }};


}  // namespace vga
}  // namespace rqdq
