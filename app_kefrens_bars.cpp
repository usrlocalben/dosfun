#include "app_kefrens_bars.hpp"

#include <cmath>
#include <string>

#include "vga_mode.hpp"
#include "vga_reg.hpp"

using std::uint8_t;

namespace rqdq {
namespace {

inline void PutPixelSlow(int x, int y, uint8_t c, uint8_t* baseAddr) {
	if (x<0 || x>=320) return;
	vga::SelectPlanes(1<<(x&3));
	baseAddr[y*80+(x>>2)] = c; }


}  // namespace
namespace app {

void DrawKefrensBars(const vga::VRAMPage dst, float T, int patternNum, int rowNum) {
	int whole = T;
	float frac = T - whole;

	const int goodLookingColorMagic[] = {
		0, 1, 2, 3, 4,
		6, 15, 18, 19, 21,
		24, 30, 33, 34, 36,
		39, 49, 51, 52 };

	uint8_t row[320];
	std::memset(row, 0, 320);

	// int magicIdx = patternNum<<1 | (rowNum>>4&1);
	int magicIdx = patternNum;
	uint8_t colorpos = goodLookingColorMagic[magicIdx%19] * 17;

	uint8_t* rowPtr = dst.addr; // + 80*60;
	for (int yyy=0; yyy<160; yyy++) {
		// animate
#define SIN std::sin
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
				row[ox] = colorpos+wx; }}

		// copy row[] to vram
		for (int p=0; p<4; p++) {
			vga::SelectPlanes(1<<p);
			for (int xxx=0; xxx<80; xxx++) {
				rowPtr[xxx] = row[xxx*4+p]; }}

		rowPtr += 80; }}


}  // namespace app
}  // namespace rqdq
