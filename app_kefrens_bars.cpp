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

#include <sys/nearptr.h>

using std::uint8_t, std::uint16_t, std::uint32_t;

//#define UNCHAINED

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

constexpr int scrWidth = 160;
constexpr int scrHeight = 120;
constexpr int halfWidth = scrWidth/2;
constexpr int scrStride = scrWidth/4;


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

#ifdef UNCHAINED
	rgl::PlanarizeLines(bkg_);
#endif
	}


void KefrensBars::Draw(const vga::VRAMPage dst, float T, int patternNum, int rowNum) {
	int whole = T;
	float frac = T - whole;


	std::array<uint8_t, scrWidth> rowData, rowMask;
	//rowData.fill(0);
	rowMask.fill(0);

	// int magicIdx = patternNum<<1 | (rowNum>>4&1);
	int magicIdx = patternNum;
	uint8_t colorpos = magicIdx%19;
#define SIN std::sin

	uint8_t* rowPtr = dst.addr + __djgpp_conventional_base;
	for (int yyy=0; yyy<scrHeight; yyy++) {
		// animate
		int pos;
		switch (patternNum%4) {
		case 0:
			pos = SIN((T*2.19343f) + yyy*(SIN(T*0.25f)*0.05f) * 3.14159f * 2.0f) * (SIN(T*1.781234f)*(halfWidth-10)) + halfWidth;
			break;
		case 1:
			pos = SIN((T) + yyy*0.005f * 3.14159f * 2.0f) * (SIN(T*3.781234f)*150) + halfWidth;
			break;
		case 2:
			pos = SIN((T*5.666f) + yyy*0.008f * 3.14159f * 2.0f) * (SIN(T*1.781234f+(yyy*0.010f))*50+100) + SIN((yyy+(T*60))*0.00898f)*100+halfWidth;
			break;
		case 3:
			pos = SIN((T*2.45f) + yyy*0.012f * 3.14159f * 2.0f) * (SIN(T*1.781234f+(yyy*0.010f))*66+33) + SIN((yyy+(T*60))*0.01111f)*100+50;
			break; }

		// draw bar
		for (int wx=-4; wx<=4; wx++) {
			int ox = pos+wx;
			auto color = glcm2[colorpos*16 + wx + 4];
			if (0 <= ox && ox < scrWidth) {
#ifdef UNCHAINED
				int plane = ox&3;
				int ofs = ox>>2;
				rowData[plane*scrStride+ofs] = color;
				rowMask[plane*scrStride+ofs] = 0xff;
#else
				rowData[ox] = color;
				rowMask[ox] = 0xff;
#endif
				}}

		// copy row[] to vram
#ifdef UNCHAINED
		for (int p=0; p<4; p++) {
			vga::Planes(1<<p);
			for (int rx=0; rx<scrStride; rx+=4) {
				auto bPx = *reinterpret_cast<uint32_t*>(bkg_.buf.data() + (yyy * bkg_.stride) + (p*80) + rx);
				auto fPx = *reinterpret_cast<uint32_t*>(rowData.data() + (p*scrStride) + rx);
				auto mask = *reinterpret_cast<uint32_t*>(rowMask.data() + (p*scrStride) + rx);
				*reinterpret_cast<uint32_t*>(rowPtr + rx) = Blend(fPx, bPx, mask); }
#ifdef SHOW_TIMING
			rowPtr[scrStride-1] = 255;
#endif
			}
		rowPtr += scrStride;
#else // UNCHAINED
		for (int rx=0; rx<scrWidth; rx+=4) {
			auto bPx = *reinterpret_cast<uint32_t*>(bkg_.buf.data() + (yyy * bkg_.stride) + rx);
			auto fPx = *reinterpret_cast<uint32_t*>(rowData.data() + rx);
			auto mask = *reinterpret_cast<uint32_t*>(rowMask.data() + rx);
			*reinterpret_cast<uint32_t*>(rowPtr + rx) = Blend(fPx, bPx, mask); }
#ifdef SHOW_TIMING
		rowPtr[scrWidth-1] = 255;
#endif
		rowPtr += scrWidth;
#endif

	}}


}  // namespace app
}  // namespace rqdq
