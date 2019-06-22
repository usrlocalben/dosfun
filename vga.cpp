#include "vga.hpp"

#include <dos.h>  // _dos_setvect, _dos_getvect
#include <i86.h>  // int386

#include "pit.hpp"

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


/*
 * VGA retrace detection.
 *
 * The VGA chipset includes an IRQ that can be raised when
 * vertical blanking begins.
 *
 * In most PC VGA implementations, it is not connected.
 *
 * Polling the status register is the only way to detect
 * vblank (or hblank).
 *
 * Detecting the _rising-edge_ of the vblank period requires
 * waiting for up to a whole frame since a vblank period may
 * already be in-progress when detection begins.
 */
inline void SpinUntilRetracing() {
	while (!(inp(VP_STA1) & VF_VRETRACE)) {}}


inline void SpinWhileRetracing() {
	while (inp(VP_STA1) & VF_VRETRACE) {}}


inline void SpinUntilNextRetraceBegins() {
	SpinWhileRetracing();
	SpinUntilRetracing(); }


/*
untested
inline void SpinUntilHorizontalRetrace() {
	while (!(inp(VP_STA1) & VF_DD)) {}}


inline void SpinWhileHorizontalRetrace() {
	while (inp(VP_STA1) & VF_DD) {}}
*/


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


void SetBIOSMode(int num) {
	union REGS r;
	r.x.eax = num;
	int386(0x10, &r, &r); }


/*
 * ModeX 320x240 initialization
 *
 * This is taken directly from M.Abrash's Mode-X .asm code
 */
void SetModeX() {
	SpinUntilRetracing();
	SetBIOSMode(0x13);

	outpw(VP_SEQC, 0x604);  // disable chain4

	outpw(VP_SEQC, 0x100);  // synchronous reset while switching clocks
	outp(VP_MISC, 0xe3);    // select 25 MHz dot clock & 60 Hz scanning rate

	outpw(VP_SEQC, 0x300);  // undo reset (restart sequencer)

	// VSync End reg contains register write-protect bit
	// get current VSync End register setting
	// remove write-protect on various CRTC registers
	outp(VP_CRTC, 0x11);
	outp(VP_CRTC+1, inp(VP_CRTC+1)&0x7f);

	outpw(VP_CRTC, 0x0d06);  // vertical total
	outpw(VP_CRTC, 0x3e07);  // overflow (bit 8 of vertical counts)
	outpw(VP_CRTC, 0x4109);  // cell height (2 to double-scan)
	outpw(VP_CRTC, 0xea10);  // v sync start
	outpw(VP_CRTC, 0xac11);  // v sync end and protect cr0-cr7
	outpw(VP_CRTC, 0xdf12);  // vertical displayed
	outpw(VP_CRTC, 0x0014);  // turn off dword mode
	outpw(VP_CRTC, 0xe715);  // v blank start
	outpw(VP_CRTC, 0x0616);  // v blank end
	outpw(VP_CRTC, 0xe317); }// turn on byte mode


}  // namespace vga
}  // namespace rqdq
