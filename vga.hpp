#pragma once
#include <cstdint>
#include <conio.h>  // outp

using std::uint8_t;
using std::uint16_t;

namespace rqdq {
namespace vga {

const int VP_SEQC = 0x3c4;  // sequence controller
const int VP_CRTC = 0x3d4;  // CRT controller
const int VP_MISC = 0x3c2;
const int VP_PALETTE_INDEX = 0x3c8;
const int VP_PALETTE_DATA = 0x3c9;
const int VP_GFXC = 0x3ce;  // graphics controller
const int VP_STA1 = 0x3da;  // Input Status #1 (color mode)
const int VF_VRETRACE = 0x08;
const int VF_DD = 0x01;

uint8_t* const VGAPTR = (uint8_t*)0xa0000L;


inline void SetRGB(int idx, int r, int g, int b) {
	outp(VP_PALETTE_INDEX, idx);
	outp(VP_PALETTE_DATA, r);
	outp(VP_PALETTE_DATA, g);
	outp(VP_PALETTE_DATA, b); }


void SetBIOSMode(int num);

const uint8_t CRT_HIGH_ADDR = 0x0c;
const uint8_t CRT_LOW_ADDR = 0x0d;

const uint8_t SC_MAP_MASK = 0x02;


inline void SetStartAddress(uint16_t addr) {
	outpw(VP_CRTC, CRT_HIGH_ADDR | (addr & 0xff00));
	outpw(VP_CRTC, CRT_LOW_ADDR | (addr << 8)); }


inline void SetBitMask(uint8_t mask) {
	outpw(VP_GFXC, mask<<8|0x08); }


inline void SelectPlanes(uint8_t mask) {
	outpw(VP_SEQC, mask<<8|SC_MAP_MASK); }


inline void PutPixelSlow(int x, int y, uint8_t c, uint8_t* baseAddr) {
	if (x<0 || x>=320) return;
	SelectPlanes(1<<(x&3));
	baseAddr[y*80+(x>>2)] = c; }


inline void SelectAllPlanes() {
	SelectPlanes(0xf); }


void SetModeX();

typedef void (*vbifunc)();

void InstallVBI(vbifunc proc);
void UninstallVBI();
float GetLastVBIFrequency();


class SoftVBI {
public:
	SoftVBI(vbifunc proc) {
		InstallVBI(proc); }
	~SoftVBI() {
		UninstallVBI(); }
	float GetFrequency() {
		return GetLastVBIFrequency(); }};


struct VRAMPage {
	const int num;
	std::uint8_t* const addr;
	const std::uint16_t vgaAddr; };


const vga::VRAMPage modeXPages[2] = {
	{ 0, vga::VGAPTR, 0 },
	{ 1, vga::VGAPTR + (320*240/4), 320*240/4 } };


}  // namespace vga
}  // namespace rqdq
