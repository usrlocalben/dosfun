#include "vga_mode.hpp"

#include <sys/nearptr.h>

#include "pc_bus.hpp"
#include "pc_cpu.hpp"
#include "vga_bios.hpp"
#include "vga_reg.hpp"

namespace rqdq {
namespace vga {

using pc::InB;
using pc::OutB;
using pc::OutW;

/*
 * ModeX 320x240 initialization
 *
 * This is taken directly from M.Abrash's Mode-X .asm code
 */
void SetModeX() {
	bios::SetMode(0x13);
	SpinUntilNextRetraceBegins();

	OutW(VP_SEQC, 0x604);  // disable chain4

	{
		pc::CriticalSection criticalSection;
		SequencerDisabledSection sequencerDisabledSection(criticalSection);
		OutB(VP_MISC, 0xe3); }  // select 25 MHz dot clock & 60 Hz scanning rate

	// VSync End reg contains register write-protect bit
	// get current VSync End register setting
	// remove write-protect on various CRTC registers
	OutB(VP_CRTC, 0x11);
	OutB(VP_CRTC+1, InB(VP_CRTC+1)&0x7f);

	OutW(VP_CRTC, 0x0d06);  // vertical total
	OutW(VP_CRTC, 0x3e07);  // overflow (bit 8 of vertical counts)
	OutW(VP_CRTC, 0x4109);  // cell height (2 to double-scan)
	OutW(VP_CRTC, 0xea10);  // v sync start
	OutW(VP_CRTC, 0xac11);  // v sync end and protect cr0-cr7
	OutW(VP_CRTC, 0xdf12);  // vertical displayed
	OutW(VP_CRTC, 0x0014);  // turn off dword mode
	OutW(VP_CRTC, 0xe715);  // v blank start
	OutW(VP_CRTC, 0x0616);  // v blank end
	OutW(VP_CRTC, 0xe317);  // turn on byte mode

	// clear ram
	SelectPlanes(0xf);
	uint8_t *dst = VGAPTR + __djgpp_conventional_base;
	for (int i=0; i<65536; i++) {
		dst[i] = 0; }}


}  // namespace vga
}  // namespace rqdq
