#pragma once
#include <cstdlib>
#include <cstdint>

#include "pc_cpu.hpp"
#include "pc_pic.hpp"
#include "pc_pit.hpp"
#include "vga_reg.hpp"

using std::uint8_t;
using std::uint16_t;

namespace rqdq {
namespace vga {

typedef void (*vbifunc)();

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
const float kJitterPct = 0.025;

const int kNumVBISamples = 50;

extern int irqSleepTimeInTicks;

template <typename VBIPROC>
class RetraceIRQ {
public:
	/**
	 * Install the IRQ system.
	 *
	 * Calibrates the timer by measuring the refresh-to-refresh
	 * time for a VGA frame using the PIT.  Then, the ISR is
	 * installed and triggered on the following frame.
	 */
	RetraceIRQ() :frameDurationInTicks_(0) {
		int ax = 0;
		for (int si=0; si<kNumVBISamples; si++) {
			SpinUntilNextRetraceBegins();
			pc::BeginMeasuring();
			SpinUntilNextRetraceBegins();
			ax += pc::EndMeasuring(); }
		frameDurationInTicks_ = ax / kNumVBISamples;

		irqSleepTimeInTicks =
			frameDurationInTicks_ * (1.0 - kJitterPct);

		SpinWhileRetracing();
		pc::pitIRQLine.SaveISR();
		pc::pitIRQLine.SetISR(RetraceIRQ::vblank_isr);
		{
			pc::CriticalSection cs;
			SpinUntilRetracing();
			pc::StartCountdown(irqSleepTimeInTicks); }}

	/**
	 * Restore the BIOS timer ISR and interval
	 */
	~RetraceIRQ() {
		pc::pitIRQLine.RestoreISR();
		pc::StartSquareWave(0); }

private:
	RetraceIRQ& operator=(const RetraceIRQ&);  // non-copyable
	RetraceIRQ(const RetraceIRQ&);          // non-copyable

public:
	float GetHz() const {
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
	static void vblank_isr() {
		/*
		 * when execution begins, retrace still hasn't started
		 */
#ifdef SHOW_TIMING
		SetRGB(0, 0x1f,0x2f,0x3f);
#endif
		vga::SpinUntilRetracing();
#ifdef SHOW_TIMING
		SetRGB(0, 0x0, 0x0, 0x0);
#endif

		/*
		 * retrace just started.
		 * reset the timer, then call the user's vbi handler
		 */
		pc::StartCountdown(irqSleepTimeInTicks);
		VBIPROC()();
		pc::pitIRQLine.SignalEOI(); }

private:
	uint16_t frameDurationInTicks_; };


}  // namespace vga
}  // namespace rqdq
