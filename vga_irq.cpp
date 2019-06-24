#include "vga_irq.hpp"

#include "pc_pit.hpp"
#include "vga_reg.hpp"

#define nullptr (0)

namespace rqdq {
namespace {

const int kNumVBISamples = 50;

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
const float kJitterPct = 0.03;

pc::IRQLine& pitIRQLine = pc::GetPITIRQLine();

int irqSleepTimeInTicks = 0;

vga::vbifunc userVBIProc = nullptr;

uint16_t frameDurationInTicks = 0;

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
void __interrupt vblank_isr() {
	/*
	 * when execution begins, retrace still hasn't started
	 */
	// SetRGB(0, 0x3f,0x3f,0x3f);
	vga::SpinUntilRetracing();
	// SetRGB(0, 0x0, 0x0, 0x0);

	/*
	 * retrace just started.
	 * reset the timer, then call the user's vbi handler
	 */
	pc::StartCountdown(irqSleepTimeInTicks);
	if (userVBIProc != nullptr) {
		userVBIProc(); }
	pitIRQLine.SignalEOI(); }

}  // namespace

namespace vga {

/**
 * Install the IRQ system.
 *
 * Calibrates the timer by measuring the refresh-to-refresh
 * time for a VGA frame using the PIT.  Then, the ISR is
 * installed and triggered on the following frame.
 */
RetraceIRQ::RetraceIRQ(vbifunc proc) {
	userVBIProc = proc;

	int ax = 0;
	for (int si=0; si<kNumVBISamples; si++) {
		SpinUntilNextRetraceBegins();
		pc::BeginMeasuring();
		SpinUntilNextRetraceBegins();
		ax += pc::EndMeasuring(); }
	frameDurationInTicks = ax / kNumVBISamples;

	irqSleepTimeInTicks =
		frameDurationInTicks * (1.0 - kJitterPct);

	SpinWhileRetracing();
	pitIRQLine.SaveVect();
	pitIRQLine.SetVect(vblank_isr);
	_disable();
	SpinUntilRetracing();
	pc::StartCountdown(irqSleepTimeInTicks);
	_enable(); }


/**
 * Restore the BIOS timer ISR and interval
 */
RetraceIRQ::~RetraceIRQ() {
	pitIRQLine.RestoreVect();
	pc::StartSquareWave(0); }


float RetraceIRQ::GetHz() const {
	return pc::ticksToHz(frameDurationInTicks); }


}  // namespace vga
}  // namespace rqdq
