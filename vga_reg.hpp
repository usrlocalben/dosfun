/*
 * VGA register-level tools
 */
#pragma once
#include "pc_bus.hpp"
#include "pc_cpu.hpp"
#include "vec.hpp"

#include <cstdint>

namespace rqdq {
namespace vga {

const int VP_SEQC = 0x3c4;  // sequence controller
const int VP_CRTC = 0x3d4;  // CRT controller
const int VP_MISC = 0x3c2;
const int VP_PALETTE_READ = 0x3c7;
const int VP_PALETTE_WRITE = 0x3c8;
const int VP_PALETTE_DATA = 0x3c9;
const int VP_GFXC = 0x3ce;  // graphics controller
const int VP_STA1 = 0x3da;  // Input Status #1 (color mode)
const int VF_VRETRACE = 0x08;
const int VF_DD = 0x01;

const std::uint8_t CRT_HIGH_ADDR = 0x0c;
const std::uint8_t CRT_LOW_ADDR = 0x0d;

const std::uint8_t SC_MAP_MASK = 0x02;

std::uint8_t* const VRAM_ADDR = (std::uint8_t*)0xa0000L;


inline void Color(int idx, rml::IVec3 c) {
	// pc::OutB(0x3c6, 0xff);
	pc::OutB(VP_PALETTE_WRITE, idx);
	pc::OutB(VP_PALETTE_DATA, c.x);
	pc::OutB(VP_PALETTE_DATA, c.y);
	pc::OutB(VP_PALETTE_DATA, c.z); }


inline rml::IVec3 Color(int idx) {
	// pc::OutB(0x3c6, 0xff);
	pc::OutB(VP_PALETTE_READ, idx);
	int r = pc::InB(VP_PALETTE_DATA);
	int g = pc::InB(VP_PALETTE_DATA);
	int b = pc::InB(VP_PALETTE_DATA);
	return { r, g, b }; }


inline rml::Vec3 ToFloat(rml::IVec3 c) {
	float r = float(c.x) / 0x3f;
	float g = float(c.y) / 0x3f;
	float b = float(c.z) / 0x3f;
	return { r, g, b }; }


inline void StartAddr(std::uint16_t addr) {
	pc::OutW(VP_CRTC, CRT_HIGH_ADDR | (addr & 0xff00));
	pc::OutW(VP_CRTC, CRT_LOW_ADDR | (addr << 8)); }


inline void MergeMask(std::uint8_t mask) {
	pc::OutW(VP_GFXC, mask<<8|0x08); }


inline int MergeMask() {
	pc::OutB(VP_GFXC, 0x08);
	return pc::InB(VP_GFXC+1); }


inline void Planes(std::uint8_t mask) {
	pc::OutW(VP_SEQC, mask<<8|SC_MAP_MASK); }


inline int Planes() {
	pc::OutB(VP_SEQC, SC_MAP_MASK);
	return pc::InB(VP_SEQC+1); }


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
	while (!(pc::InB(VP_STA1) & VF_VRETRACE)) {}}


inline void SpinWhileRetracing() {
	while (pc::InB(VP_STA1) & VF_VRETRACE) {}}


inline void SpinUntilNextRetraceBegins() {
	SpinWhileRetracing();
	SpinUntilRetracing(); }


/*
untested
inline void SpinUntilHorizontalRetrace() {
	while (!(pc::InB(VP_STA1) & VF_DD)) {}}


inline void SpinWhileHorizontalRetrace() {
	while (pc::InB(VP_STA1) & VF_DD) {}}
*/


class SequencerDisabledSection {
public:
	SequencerDisabledSection(const pc::CriticalSection& cs) :cs(cs) {
		pc::OutW(VP_SEQC, 0x100); }  // clear bit 1, starting reset
	~SequencerDisabledSection() {
		pc::OutW(VP_SEQC, 0x300); }  // undo reset / restart sequencer)
private:
	SequencerDisabledSection& operator=(const SequencerDisabledSection&);  // non-copyable
	SequencerDisabledSection(const SequencerDisabledSection&);             // non-copyable
	const class pc::CriticalSection& cs; };


}  // namespace vga
}  // namespace rqdq
