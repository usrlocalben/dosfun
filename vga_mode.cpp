#include "vga_mode.hpp"

#include "pc_bus.hpp"
#include "pc_cpu.hpp"
#include "vga_bios.hpp"
#include "vga_reg.hpp"

namespace rqdq {
namespace vga {

/*
 * ModeX 320x240 initialization
 *
 * This is taken directly from M.Abrash's Mode-X .asm code
 */
void SetModeX() {
	bios::SetMode(0x13);
	SpinUntilNextRetraceBegins();

	pc::OutW(VP_SEQC, 0x604);  // disable chain4

	{
		pc::CriticalSection criticalSection;
		SequencerDisabledSection sequencerDisabledSection(criticalSection);
		pc::OutB(VP_MISC, 0xe3); }  // select 25 MHz dot clock & 60 Hz scanning rate

	// VSync End reg contains register write-protect bit
	// get current VSync End register setting
	// remove write-protect on various CRTC registers
	pc::OutB(VP_CRTC, 0x11);
	pc::OutB(VP_CRTC+1, pc::InB(VP_CRTC+1)&0x7f);

	pc::OutW(VP_CRTC, 0x0d06);  // vertical total
	pc::OutW(VP_CRTC, 0x3e07);  // overflow (bit 8 of vertical counts)
	pc::OutW(VP_CRTC, 0x4109);  // cell height (2 to double-scan)
	pc::OutW(VP_CRTC, 0xea10);  // v sync start
	pc::OutW(VP_CRTC, 0xac11);  // v sync end and protect cr0-cr7
	pc::OutW(VP_CRTC, 0xdf12);  // vertical displayed
	pc::OutW(VP_CRTC, 0x0014);  // turn off dword mode
	pc::OutW(VP_CRTC, 0xe715);  // v blank start
	pc::OutW(VP_CRTC, 0x0616);  // v blank end
	pc::OutW(VP_CRTC, 0xe317);  // turn on byte mode

	// clear ram
	SelectPlanes(0xf);
	for (int i=0; i<65536; i++) {
		VGAPTR[i] = 0; }}


}  // namespace vga
}  // namespace rqdq
