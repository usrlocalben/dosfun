#include "vga.hpp"

#include <dos.h>  // _dos_setvect, _dos_getvect
#include <i86.h>  // int386

#include "pit.hpp"

#define nullptr (0)

namespace rqdq {
namespace {

const int kNumVBISamples = 50;

const float kSoftVBIJitterPct = 0.05;

}  // namespace

namespace vga {

inline void SpinUntilRetracing() {
	while (!(inp(VP_STA1) & VF_VRETRACE)) {}}


inline void SpinWhileRetracing() {
	while (inp(VP_STA1) & VF_VRETRACE) {}}


inline void SpinUntilNextRetraceBegins() {
	SpinWhileRetracing();
	SpinUntilRetracing(); }


/*
inline void SpinUntilHorizontalRetrace() {
	while (!(inp(VP_STA1) & VF_DD)) {}}


inline void SpinWhileHorizontalRetrace() {
	while (inp(VP_STA1) & VF_DD) {}}
*/


void SetBIOSMode(int num) {
	union REGS r;
	r.x.eax = num;
	int386(0x10, &r, &r); }


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


void (__interrupt * oldTimerISRPtr)();

const int TIMER_ISR_NUM = 0x08;

volatile uint16_t softVBISleepTimeInTicks = 0;

volatile vbifunc userVBIProc = nullptr;

void __interrupt vblank_isr() {
	// SetRGB(0, 0x3f,0x3f,0x3f);
	SpinUntilRetracing();
	// SetRGB(0, 0x0, 0x0, 0x0);
	pit::StartCountdown(softVBISleepTimeInTicks);
	if (userVBIProc != nullptr) {
		userVBIProc(); }
	outp(0x20, 0x20); }

uint16_t approximateFrameDurationInTicks = 0;


void InstallVBI(vbifunc proc) {
	oldTimerISRPtr = _dos_getvect(TIMER_ISR_NUM);

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
	_dos_setvect(TIMER_ISR_NUM, &vblank_isr);
	SpinUntilRetracing();
	pit::StartCountdown(softVBISleepTimeInTicks); }


void UninstallVBI() {
	_dos_setvect(TIMER_ISR_NUM, oldTimerISRPtr);
	pit::StartSquareWave(0); }


float GetLastVBIFrequency() {
	return pit::ticksToHz(approximateFrameDurationInTicks); }


}  // namespace vga
}  // namespace rqdq
