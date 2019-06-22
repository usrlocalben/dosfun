#include "vga_mode.hpp"

#include <i86.h>  // int386

#include "vga_reg.hpp"

namespace rqdq {
namespace vga {


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
