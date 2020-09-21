#include "vga_mode.hpp"

#include "pc_bus.hpp"
#include "pc_cpu.hpp"
#include "vga_bios.hpp"
#include "vga_reg.hpp"

#include <sys/nearptr.h>

using rqdq::pc::InB;
using rqdq::pc::OutB;
using rqdq::pc::OutW;
using std::uint8_t;
using std::uint16_t;

namespace rqdq {
namespace {

using namespace vga;

class AttributeControllerSection {
public:
	AttributeControllerSection() {}
	~AttributeControllerSection() {
		// restore PAS=1 "for normal operation"
		OutB(0x3c0, 0x20);}};


class TimingRegisterSection {
public:
	/**
	 * VSync End reg contains register write-protect bit
	 * get current VSync End register setting
	 * remove write-protect on various CRTC registers
	 */
	TimingRegisterSection() {
		OutB(VP_CRTC, 0x11);
		OutB(VP_CRTC+1, InB(VP_CRTC+1)&0x7f); }

	~TimingRegisterSection() {
		OutB(VP_CRTC, 0x11);
		OutB(VP_CRTC+1, InB(VP_CRTC+1)|0x80); }};


void crtc(uint8_t idx, uint8_t a) {
	OutW(VP_CRTC, (uint16_t(a)<<8)|idx); }

void crtc_and_or(uint8_t idx, uint8_t andbits, uint8_t orbits) {
	OutB(VP_CRTC, idx);
	OutB(VP_CRTC+1, (InB(VP_CRTC+1)&andbits)|orbits); }

void seqc_and_or(uint8_t idx, uint8_t andbits, uint8_t orbits) {
	OutB(VP_SEQC, idx);
	OutB(VP_SEQC+1, (InB(VP_SEQC+1)&andbits)|orbits); }

void gfxc_and_or(uint8_t idx, uint8_t andbits, uint8_t orbits) {
	OutB(VP_GFXC, idx);
	OutB(VP_GFXC+1, (InB(VP_GFXC+1)&andbits)|orbits); }

void misc_and_or(uint8_t andbits, uint8_t orbits) {
	OutB(VP_MISC, (InB(0x3cc)&andbits)|orbits); }

}  // close unnamed namespace
namespace vga {

/**
 * ModeX 320x240 initialization
 *
 * This is taken directly from M.Abrash's Mode-X .asm code
 */
void ModeX() {
	bios::Mode(0x13);
	SpinUntilNextRetraceBegins();

	OutW(VP_SEQC, 0x604);  // disable chain4

	{
		pc::CriticalSection criticalSection;
		SequencerDisabledSection sequencerDisabledSection(criticalSection);
		OutB(VP_MISC, 0xe3); }  // select 25 MHz dot clock & 60 Hz scanning rate

	{
		TimingRegisterSection trs;
		OutW(VP_CRTC, 0x0d06);  // vertical total
		OutW(VP_CRTC, 0x3e07);  // overflow (bit 8 of vertical counts)
		OutW(VP_CRTC, 0x4109);  // repeat scan lines 2x
		OutW(VP_CRTC, 0xea10);  // v sync start
		OutW(VP_CRTC, 0xac11);  // v sync end and protect cr0-cr7
		OutW(VP_CRTC, 0xdf12);  // vertical displayed
		OutW(VP_CRTC, 0x0014);  // turn off dword mode
		OutW(VP_CRTC, 0xe715);  // v blank start
		OutW(VP_CRTC, 0x0616);  // v blank end
		OutW(VP_CRTC, 0xe317); }// turn on byte mode

	// clear ram
	Planes(0xf);
	uint8_t *dst = VRAM_ADDR + __djgpp_conventional_base;
	for (int i=0; i<65536; i++) {
		dst[i] = 0; }}



/**
 * 
 */
void Mode160() {
	bios::Mode(0x0D); // ???

	{
		TimingRegisterSection trs;
		// Increment "Start Horizontal Retrace"
		OutB(VP_CRTC, 0x04);
		OutB(VP_CRTC+1, InB(VP_CRTC+1)+1);}

	// read VP_MISC at 0x3cc
	// set VSYNCP=1 (bit 7)
	// set HSYNCP=0 (bit 6)
	// bit0-5 untouched
	misc_and_or(0b0011'1111, 0x80);

	{
		TimingRegisterSection trs;
		crtc(0x06, 0x0b);  // vertical total?????
		crtc(0x07, 0x3e);  // overflow (bit 8 of vertical counts)
		crtc(0x10, 0xea);  // v sync start
		crtc(0x11, 0x8c);  // v sync end ?????
		crtc(0x12, 0xdf);  // vertical displayed
		crtc(0x15, 0xe7);  // v blank start
		crtc(0x16, 0x04);} // v blank end ?????

	// "Graphics Mode Register"
	// set Shift256,
	// pass-thru "Shift Register Interleave Mode"
	// clear the rest
	// pretty weird!
	gfxc_and_or(0x05, 0x60, 0x40);

	// read the retrace (input status register 1)
	// but why?
	// InB(VP_STA1);

	{
		AttributeControllerSection acs;
		// set 8BIT=1 in the "Attribute mode Control Register"
		OutB(0x3c0, 0x30);  // PAS=1, addr=0x10
		OutB(0x3c0, InB(0x3c1)|0x40); 

		for (int c=0; c<16; ++c) {
			OutB(0x3c0, c);  // select color #c
			OutB(0x3c0, c);}}// map to index #c

	// OutB(VP_PALETTE_WRITE, 0);

	seqc_and_or(0x04, 0xff, 0x08);  // enable chain-4 in "sequencer memory mode register"
	crtc_and_or(0x14, 0xff, 0x40);  // set Double-Word addressing in "Underline Location Register"
	crtc_and_or(0x17, 0xff, 0x40);  // set Word/Byte mode select to 1, byte mode?
	crtc_and_or(0x09, 0x60, 0x03);} // repeat scan lines 4x (3+1)


}  // namespace vga
}  // namespace rqdq
