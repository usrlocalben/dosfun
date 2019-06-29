#include "vga_mode.hpp"

#include <i86.h>  // int386

#include "pc_bus.hpp"
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

	pc::TXdw(VP_SEQC, 0x604);  // disable chain4

	pc::TXdw(VP_SEQC, 0x100);  // synchronous reset while switching clocks
	pc::TXdb(VP_MISC, 0xe3);    // select 25 MHz dot clock & 60 Hz scanning rate

	pc::TXdw(VP_SEQC, 0x300);  // undo reset (restart sequencer)

	// VSync End reg contains register write-protect bit
	// get current VSync End register setting
	// remove write-protect on various CRTC registers
	pc::TXdb(VP_CRTC, 0x11);
	pc::TXdb(VP_CRTC+1, pc::RXdb(VP_CRTC+1)&0x7f);

	pc::TXdw(VP_CRTC, 0x0d06);  // vertical total
	pc::TXdw(VP_CRTC, 0x3e07);  // overflow (bit 8 of vertical counts)
	pc::TXdw(VP_CRTC, 0x4109);  // cell height (2 to double-scan)
	pc::TXdw(VP_CRTC, 0xea10);  // v sync start
	pc::TXdw(VP_CRTC, 0xac11);  // v sync end and protect cr0-cr7
	pc::TXdw(VP_CRTC, 0xdf12);  // vertical displayed
	pc::TXdw(VP_CRTC, 0x0014);  // turn off dword mode
	pc::TXdw(VP_CRTC, 0xe715);  // v blank start
	pc::TXdw(VP_CRTC, 0x0616);  // v blank end
	pc::TXdw(VP_CRTC, 0xe317);  // turn on byte mode

	// clear ram
	SelectPlanes(0xf);
	for (int i=0; i<65536; i++) {
		VGAPTR[i] = 0; }}


}  // namespace vga
}  // namespace rqdq
