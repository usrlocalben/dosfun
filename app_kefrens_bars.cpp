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

using std::uint8_t;

namespace rqdq {
namespace {

/**
 * select bits in a where selector=1 else b
 * out[n] = sel[n] ? a[n] : b[n]
 */
inline uint32_t Blend(uint32_t a, uint32_t b, uint32_t sel) {
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

std::array<uint8_t, 32*32> tileData;

}  // namespace
namespace app {

KefrensBars::KefrensBars() {
	std::vector<rml::Vec3> vgaPal(256);
	for (int i=0; i<256; i++) {
		vgaPal[i] = rgl::ToLinear(vga::ToFloat(vga::Color(i))); }

	std::vector<rml::IVec3> newPal;
	std::tie(bkg_, newPal) = Reindex(rgl::LoadPNG(data::amy));

	for (int i=0; i<256; i++) {
		vga::Color(i, { newPal[i].x, newPal[i].y, newPal[i].z }); }

	for (int i=0; i<19; i++) {
		for (int c=-4; c<=4; c++) {
			int idx = ((goodLookingColorMagic[i]*17) + c) & 0xff;
			const auto oldColor = vgaPal[idx];

			float minDist = 9999999.0f;
			int minIdx = -1;
			for (int pi=0; pi<255; pi++) {
				auto testColor = rgl::ToLinear(vga::ToFloat(newPal[pi]));
				auto dist = Length(testColor - oldColor);
				if (dist < minDist) {
					minDist = dist;
					minIdx = pi; }}
			glcm2[i * 16 + c + 4] = minIdx; }}

	for (int y=0; y<32; y++) {
		for (int x=0; x<32; x++) {
			rml::Vec3 color{ (y+1)/32.0f, (x+1)/32.0f, 0.5f };

			float minDist = 9999999.0f;
			int minIdx = -1;
			for (int pi=0; pi<254; pi++) {
				auto testColor = rgl::ToLinear(vga::ToFloat(newPal[pi]));
				auto dist = Length(testColor - color);
				if (dist < minDist) {
					minDist = dist;
					minIdx = pi; }}
			tileData[y*32+x] = minIdx; }}

	rgl::PlanarizeLines(bkg_); }


void KefrensBars::Draw(const vga::VRAMPage dst, float T, int patternNum, int rowNum) {
	int whole = T;
	float frac = T - whole;


	/*
	std::array<uint8_t, 320> rowData, rowMask;
	//rowData.fill(0);
	rowMask.fill(0);

	// int magicIdx = patternNum<<1 | (rowNum>>4&1);
	int magicIdx = patternNum;
	uint8_t colorpos = magicIdx%19;
#define SIN std::sin

	// rgl::StoreTile({ 64, 64 }, tileData.data(), bkg_.buf.data());

	uint8_t* rowPtr = dst.addr;
	for (int yyy=0; yyy<240; yyy++) {
		// animate
		int pos;
		switch (patternNum%4) {
		case 0:
			pos = SIN((T*2.19343f) + yyy*(SIN(T*0.25f)*0.05f) * 3.14159f * 2.0f) * (SIN(T*1.781234f)*150) + 160;
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

		// draw bar
		for (int wx=-4; wx<=4; wx++) {
			int ox = pos+wx;
			if (0 <= ox && ox < 320) {
				int plane = ox&3;
				int ofs = ox>>2;
				rowData[plane*80+ofs] = glcm2[colorpos*16 + wx + 4];
				rowMask[plane*80+ofs] = 0xff; }}

		// copy row[] to vram planes
		for (int p=0; p<4; p++) {
			vga::Planes(1<<p);
			for (int rx=0; rx<80; rx+=4) {
				auto bPx = *reinterpret_cast<uint32_t*>(bkg_.buf.data() + (yyy * bkg_.stride) + (p*80) + rx);
				auto fPx = *reinterpret_cast<uint32_t*>(rowData.data() + (p*80) + rx);
				auto mask = *reinterpret_cast<uint32_t*>(rowMask.data() + (p*80) + rx);
				*reinterpret_cast<uint32_t*>(rowPtr + rx) = Blend(fPx, bPx, mask); }
#ifdef SHOW_TIMING
			rowPtr[79] = 255;
#endif
			}

		rowPtr += 80; }
	*/

	int a = T*2.75f;
	uint8_t* rowPtr = dst.addr;
	for (int ty=0; ty<7; ty++) {
		for (int tx=0; tx<10; tx++) {
			std::memset(rgl::tileColor.data(), 0, 32*32);
			std::memset(rgl::tileDepth.data(), 0, 32*32*2);
			for (int i=0; i<32*32; i++) {
				rgl::tileDepth[i]++;
				rgl::tileColor[i] = ty+tx+a; }
			rgl::StoreTile({ tx*32, ty*32 }, rgl::tileColor.data(), dst); }
#ifdef SHOW_TIMING
		vga::Planes(0xf);
		for (int ry=0; ry<32; ry++) {
			rowPtr[79] = 255;
			rowPtr += 80; }
#endif
		}

	}


}  // namespace app
}  // namespace rqdq
