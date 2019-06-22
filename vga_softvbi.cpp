#include "vga_softvbi.hpp"

#include "pit.hpp"
#include "vga_reg.hpp"

#define nullptr (0)

namespace rqdq {
namespace {

const int kNumVBISamples = 50;

/**
 * Soft-VBI PIT jitter allowance, as a percentage of
 * frame-time.
 *
 * The smaller this value is, the less time will be
 * consumed spinning before retrace begins.
 *
 * However, if this value is _too small_, timing jitter may
 * cause the ISR to be called _after_ retrace has already
 * started.
 *
 * This would be disastrous: the effective soft-VBI period
 * would be longer than the VGA frame.  The IRQ-raise time
 * would drift slightly with each successive frame,
 * eventually being raised _after_ retrace causing the cpu
 * to spin for an entire display period.  Additionally, the
 * wall-clock timer provided by the Soft-VBI would miss a
 * tick.
 */
const float kSoftVBIJitterPct = 0.03;


}  // namespace

namespace vga {

pic::IRQLine pitIRQLine = pit::make_irqline();

int softVBISleepTimeInTicks = 0;

vbifunc userVBIProc = nullptr;

uint16_t approximateFrameDurationInTicks = 0;


/**
 * Soft-VBI ISR
 *
 * Execution should begin _just before_ VGA blanking/retrace
 * begins.
 *
 * The CPU will spin until the rising-edge of the retrace
 * signal is detected, then immediately begin the next PIT
 * countdown before calling the user's VBI function.
 *
 * See also kSoftVBIJitterPct
 */
void __interrupt vblank_isr() {
	/*
	 * when execution begins, retrace still hasn't started
	 */
	// SetRGB(0, 0x3f,0x3f,0x3f);
	SpinUntilRetracing();
	// SetRGB(0, 0x0, 0x0, 0x0);

	/*
	 * retrace just started.
	 * reset the timer, then call the soft vbi handler
	 */
	pit::StartCountdown(softVBISleepTimeInTicks);
	if (userVBIProc != nullptr) {
		userVBIProc(); }
	pitIRQLine.SignalEOI(); }


/**
 * Install the Soft-VBI system.
 *
 * Calibrates the Soft-VBI timer by measuring the
 * refresh-to-refresh time for a VGA frame using the PIT.
 * Then, the ISR is installed and triggered on the
 * following frame.
 */
void InstallVBI(vbifunc proc) {
	userVBIProc = proc;

	int ax = 0;
	for (int si=0; si<kNumVBISamples; si++) {
		SpinUntilNextRetraceBegins();
		pit::BeginMeasuring();
		SpinUntilNextRetraceBegins();
		ax += pit::EndMeasuring(); }
	approximateFrameDurationInTicks = ax / kNumVBISamples;

	softVBISleepTimeInTicks =
		approximateFrameDurationInTicks * (1.0 - kSoftVBIJitterPct);

	SpinWhileRetracing();
	pitIRQLine.SaveVect();
	pitIRQLine.SetVect(vblank_isr);
	SpinUntilRetracing();
	pit::StartCountdown(softVBISleepTimeInTicks); }


/**
 * Restore the BIOS timer ISR and interval
 */
void UninstallVBI() {
	pitIRQLine.RestoreVect();
	pit::StartSquareWave(0); }


float GetLastVBIFrequency() {
	return pit::ticksToHz(approximateFrameDurationInTicks); }


}  // namespace vga
}  // namespace rqdq
