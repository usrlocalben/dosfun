#include "app_kefrens_bars.hpp"

#include "data_amy.hpp"
#include "canvas.hpp"
#include "picopng.hpp"
#include "vec.hpp"
#include "vga_mode.hpp"
#include "vga_reg.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <iostream>

#include <sys/nearptr.h>

using std::uint8_t;

namespace rqdq {
namespace {

/**
 * select bits in a where selector=1 else b
 * out[n] = sel[n] ? a[n] : b[n]
 */
inline
auto Blend(uint32_t a, uint32_t b, uint32_t sel) -> uint32_t {
	// neither clang nor msvc will
	// optimize this into xor/xor/and
	// return (a&sel)|(b&~sel);
	return b^(a^b)&sel; }


const int goodLookingColorMagic[] = {
	0, 1, 2, 3, 4,
	6, 15, 18, 19, 21,
	24, 30, 33, 34, 36,
	39, 49, 51, 52 };


std::array<char, 19*16> glcm2;

constexpr int sintabsiz = 128;
std::array<float, sintabsiz> sintab;
std::array<int32_t, sintabsiz> sintabfx;

constexpr auto kPi = 3.14159265358979F;

struct flub { flub() {
	for (int i=0; i<sintabsiz; ++i) {
		auto r = float(i)*2*kPi / sintabsiz;
		sintabfx[i] = int(sin(r) * 65536.0F);
		sintab[i] = sin(r); }}} flubi;

constexpr float rad2idx = sintabsiz / (2.0F*kPi);

auto tsin(float r) -> float {
	//auto idx = static_cast<std::uint8_t>(r*rad2idx);
	auto idx = static_cast<std::int32_t>(r*rad2idx);
	idx &= (sintabsiz-1);
	return sintab[idx]; }

auto tsinfx(int32_t r) -> float {
	//auto idx = static_cast<std::uint8_t>(r*rad2idx);
	auto idx = static_cast<std::int32_t>(r*rad2idx);
	r >>= 16;
	r &= (sintabsiz-1);
	return sintabfx[idx]; }

auto f2fx(float a) -> int32_t {
	return a * 65536.0F; };

auto mulfx(int32_t a, int32_t b) -> int32_t {
	return ((int64_t)a * b) >> 16; }

}  // namespace
namespace app {

KefrensBars::KefrensBars() {
	std::vector<rml::Vec3> vgaPal(256);
	for (int i=0; i<256; i++) {
		vgaPal[i] = vga::Color(i).Vec3(); }

	std::vector<rgl::TrueColorPixel> newPal;
	std::tie(bkg_, newPal) = Reindex(rgl::LoadPNG(data::amy));

	for (int i=0; i<256; i++) {
		vga::Color(i, newPal[i]); }

	std::vector<rml::Vec3> tmp(256);
	for (int i=0; i<256; ++i) {
		tmp[i] = newPal[i].Vec3(); }

	auto NearestColor = [&tmp](rml::Vec3 c) {
		float minDist = 9999999.0f;
		int bestIdx = -1;
		for (int i=0; i<255; ++i) {
			auto dist = Length(tmp[i] - c);
			if (Length(tmp[i] - c) < minDist) {
				minDist = dist;
				bestIdx = i; }}
		return bestIdx; };

	for (int i=0; i<19; i++) {
		for (int c=-4; c<=4; c++) {
			int idx = ((goodLookingColorMagic[i]*17) + c) & 0xff;
			glcm2[i*16 + c + 4] = NearestColor(vgaPal[idx]); }}

	rgl::PlanarizeLines(bkg_); }

constexpr auto kStride = int{80};
constexpr auto kWidth = int{320};
constexpr auto kHeight = int{240};


void KefrensBars::Draw(const vga::VRAMPage dst, const float T, const int patternNum, const int rowNum) {
	int whole = T;
	float frac = T - whole;

	alignas(16) std::array<uint8_t, kWidth> rowData;
	alignas(16) std::array<uint8_t, kWidth> rowMask;
	rowMask.fill(0);

	// int magicIdx = patternNum<<1 | (rowNum>>4&1);
	int magicIdx = patternNum;
	uint8_t colorpos = magicIdx%19;
#define SIN std::sin
// #define SIN tsin

	uint8_t* destRow = dst.addr + __djgpp_conventional_base;
	uint8_t* imgRow = bkg_.buf.data();

	int yyy;
	for (yyy=0; yyy<50; yyy++) {
		// animate
		int pos = SIN((T*2.19343f) + yyy*(SIN(T*0.25f)*0.05f) * 3.14159f * 2.0f) * (SIN(T*1.781234f)*150);
		pos += kWidth/2;

		// draw bar
		if (0 <= pos-4 && pos+4 < kWidth) {
			// no clipping needed
			for (int wx=-4; wx<=4; wx++) {
				int ox = pos+wx;
				int plane = ox&3;
				int ofs = ox>>2;
				rowData[plane*kStride+ofs] = glcm2[colorpos*16 + wx + 4];
				rowMask[plane*kStride+ofs] = 0xff; }}
		else {
			// need clipping
			for (int wx=-4; wx<=4; wx++) {
				int ox = pos+wx;
				if (0 <= ox && ox < kWidth) {
					int plane = ox&3;
					int ofs = ox>>2;
					rowData[plane*kStride+ofs] = glcm2[colorpos*16 + wx + 4];
					rowMask[plane*kStride+ofs] = 0xff; }}}

		// copy row[] to vram planes
		for (int p=0; p<4; p++) {
			vga::Planes(1<<p);
			for (int rx=0; rx<kStride; rx+=4) {
				auto bPx = *reinterpret_cast<uint32_t*>(imgRow + (p*kStride) + rx);
				auto fPx = *reinterpret_cast<uint32_t*>(rowData.data() + (p*kStride) + rx);
				auto mask = *reinterpret_cast<uint32_t*>(rowMask.data() + (p*kStride) + rx);
				*reinterpret_cast<uint32_t*>(destRow + rx) = Blend(fPx, bPx, mask); }
#ifdef SHOW_TIMING
			destRow[79] = 255;
#endif
			}

		imgRow += bkg_.stride;
		destRow += kStride; }

	for (; yyy<kHeight; ++yyy) {
		for (int p=0; p<4; p++) {
			vga::Planes(1<<p);
			for (int rx=0; rx<kStride; rx+=4) {
				auto bPx = *reinterpret_cast<uint32_t*>(imgRow + (p*kStride) + rx);
				auto fPx = *reinterpret_cast<uint32_t*>(rowData.data() + (p*kStride) + rx);
				auto mask = *reinterpret_cast<uint32_t*>(rowMask.data() + (p*kStride) + rx);
				*reinterpret_cast<uint32_t*>(destRow + rx) = Blend(fPx, bPx, mask); }
#ifdef SHOW_TIMING
			destRow[79] = 255;
#endif
			}

		imgRow += bkg_.stride;
		destRow += kStride; }}


}  // namespace app
}  // namespace rqdq

		/*
		switch (patternNum%4) {
		case 0:
			pos = SIN((T*2.19343f) + yyy*(SIN(T*0.25f)*0.05f) * 3.14159f * 2.0f) * (SIN(T*1.781234f)*150);
			break;
		case 1:
			pos = SIN((T) + yyy*0.005f * 3.14159f * 2.0f) * (SIN(T*3.781234f)*150) + 160;
			break;
		case 2:
			pos = SIN((T*5.666f) + yyy*0.008f * 3.14159f * 2.0f) * (SIN(T*1.781234f+(yyy*0.010f))*50+100) + SIN((yyy+(T*60))*0.00898f)*100+160;
			break;
		case 3:
			pos = SIN((T*2.45f) + yyy*0.012f * 3.14159f * 2.0f) * (SIN(T*1.781234f+(yyy*0.010f))*66+33) + SIN((yyy+(T*60))*0.01111f)*100+50;
			break; }
			*/
