#include "vga_mode.hpp"

#include "log.hpp"
#include "pc_bus.hpp"
#include "pc_cpu.hpp"
#include "vga_bios.hpp"
#include "vga_reg.hpp"

#include <stdexcept>
#include <sys/nearptr.h>

using rqdq::pc::InB;
using rqdq::pc::OutB;
using rqdq::pc::OutW;

namespace rqdq {
namespace {

using namespace rqdq::vga;

/*
 * ModeX 320x240 initialization
 *
 * This is taken directly from M.Abrash's Mode-X .asm code
 */
void ModeX() {
	BIOSUtil::Mode(0x13);
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
	Planes(0xf);
	uint8_t *dst = VRAM_ADDR + __djgpp_conventional_base;
	for (int i=0; i<65536; i++) {
		dst[i] = 0; }}

}  // close unnamed namespace

namespace vga {

ModeSetter::ModeSetter() :
	oldMode_(BIOSUtil::Mode()) {}


ModeSetter::~ModeSetter() {
	if (width_ > 0) {
		log::info("vga: restoring prior mode %02x", oldMode_);
		BIOSUtil::Mode(oldMode_); }}


void ModeSetter::Set(int width, int height, bool linear) {
	if (width==320 && height==200 && linear) {
		BIOSUtil::Mode(0x13);
		log::info("vga: set 320x200x250 linear (via BIOS mode 13h)");
		width_ = 320;
		height_ = 200;
		stride_ = 320;
		linear_ = true; }
	else if (width==320 && height==240 && !linear) {
		ModeX();
		log::info("vga: set 320x240x256 planar");
		width_ = 320;
		height_ = 240;
		stride_ = 80;
		linear_ = false; }
	else {
		throw std::runtime_error("bad mode!");}}


}  // close package namespace
}  // close enterprise namespace
