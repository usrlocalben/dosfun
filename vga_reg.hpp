/*
 * VGA register-level tools
 */
#pragma once
#include "pc_bus.hpp"
#include "pc_cpu.hpp"
#include "pixel.hpp"

#include <cstdint>

namespace rqdq {
namespace vga {

constexpr int VP_SEQC = 0x3c4;  // sequence controller
constexpr int VP_CRTC = 0x3d4;  // CRT controller
constexpr int VP_MISC = 0x3c2;
constexpr int VP_PALETTE_READ = 0x3c7;
constexpr int VP_PALETTE_WRITE = 0x3c8;
constexpr int VP_PALETTE_DATA = 0x3c9;
constexpr int VP_GFXC = 0x3ce;  // graphics controller
constexpr int VP_STA1 = 0x3da;  // Input Status #1 (color mode)
constexpr int VF_VRETRACE = 0x08;
constexpr int VF_DD = 0x01;

constexpr std::uint8_t CRT_HIGH_ADDR = 0x0c;
constexpr std::uint8_t CRT_LOW_ADDR = 0x0d;

constexpr std::uint8_t SC_MAP_MASK = 0x02;

static std::uint8_t* const VRAM_ADDR = reinterpret_cast<std::uint8_t*>(0xa0000U);


inline
void Color(int idx, rgl::TrueColorPixel c) {
	// pc::OutB(0x3c6, 0xff);
	pc::OutB(VP_PALETTE_WRITE, idx);
	pc::OutB(VP_PALETTE_DATA, c.r>>2);
	pc::OutB(VP_PALETTE_DATA, c.g>>2);
	pc::OutB(VP_PALETTE_DATA, c.b>>2); }


inline
auto Color(int idx) -> rgl::TrueColorPixel {
	// pc::OutB(0x3c6, 0xff);
	pc::OutB(VP_PALETTE_READ, idx);
	rgl::TrueColorPixel px;
	px.r = pc::InB(VP_PALETTE_DATA) << 2;
	px.g = pc::InB(VP_PALETTE_DATA) << 2;
	px.b = pc::InB(VP_PALETTE_DATA) << 2;
	return px; }


inline
void StartAddr(std::uint16_t addr) {
	pc::OutW(VP_CRTC, CRT_HIGH_ADDR | (addr & 0xff00));
	pc::OutW(VP_CRTC, CRT_LOW_ADDR | (addr << 8)); }


inline
void MergeMask(std::uint8_t mask) {
	pc::OutW(VP_GFXC, mask<<8|0x08); }


inline
auto MergeMask() -> int {
	pc::OutB(VP_GFXC, 0x08);
	return pc::InB(VP_GFXC+1); }


inline
void Planes(std::uint8_t mask) {
	pc::OutW(VP_SEQC, mask<<8|SC_MAP_MASK); }


inline
auto Planes() -> int {
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
inline
void SpinUntilRetracing() {
	while (!(pc::InB(VP_STA1) & VF_VRETRACE)) {}}


inline
void SpinWhileRetracing() {
	while (pc::InB(VP_STA1) & VF_VRETRACE) {}}


inline
void SpinUntilNextRetraceBegins() {
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
	const class pc::CriticalSection& cs;
public:
	SequencerDisabledSection(const pc::CriticalSection& cs) :
		cs(cs) {
		pc::OutW(VP_SEQC, 0x100); }  // clear bit 1, starting reset
	~SequencerDisabledSection() {
		pc::OutW(VP_SEQC, 0x300); }  // undo reset / restart sequencer)

	// non-copyable
	auto operator=(const SequencerDisabledSection&) -> SequencerDisabledSection& = delete;
	SequencerDisabledSection(const SequencerDisabledSection&) = delete; };


}  // namespace vga
}  // namespace rqdq
