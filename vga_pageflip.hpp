#pragma once
#include "vga_mode.hpp"
#include "vga_reg.hpp"

#include <cstdint>

namespace rqdq {
namespace vga {

extern int timeInFrames;
extern int backPage;
extern bool backLocked;
extern int paletteStart;
extern int paletteCnt;
extern std::uint8_t paletteBuffer[768];
extern std::uint8_t* paletteData;


struct FlipPages {
	void operator()() {
		++timeInFrames;
		if (!backLocked) {
			if (paletteCnt > 0) {
				Color(paletteStart, paletteCnt, paletteData);
				paletteCnt = 0; }
			backPage ^= 1;
			backLocked = true; }}};


class DrawContext {

	bool locked_;
	int nextStartAddr_{0};

public:
	DrawContext() :
		locked_(backLocked) {}

	// not copyable
	DrawContext(const DrawContext&) = delete;
	auto operator=(const DrawContext&) -> DrawContext& = delete;

	~DrawContext() {
		Unlock(); }

public:
	auto Get() const -> int {
		return backPage; }

	void StartAddr(int x) {
		nextStartAddr_ = x; }

	auto IsLocked() const -> bool {
		return locked_; }

	void Palette(int start, int len, rgl::TrueColorPixel* c) {
		paletteStart = start;
		paletteCnt = len*3;
		paletteData = paletteBuffer;
		for (int i=0; i<len; ++i) {
			paletteData[i*3+0] = c[i].r>>2;
			paletteData[i*3+1] = c[i].g>>2;
			paletteData[i*3+2] = c[i].b>>2; }}

	void Palette(int start, int len, std::uint8_t* data) {
		paletteStart = start;
		paletteCnt = len*3;
		paletteData = data; }

	void Unlock() {
		if (locked_) {
			StartAddr(nextStartAddr_);
			locked_ = false;
			backLocked = false; }}};


inline
auto GetTime() -> int {
	return timeInFrames; }


}  // namespace vga
}  // namespace rqdq
