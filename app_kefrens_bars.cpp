#include "app_kefrens_bars.hpp"

#include <cmath>

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

	// int magicIdx = patternNum<<1 | (rowNum>>4&1);
	int magicIdx = patternNum;
	uint8_t colorpos = goodLookingColorMagic[magicIdx%19] * 17;

	bool first = true;
	uint8_t* rowPtr = dst.addr;
	uint8_t* prevPtr = dst.addr - 80;
	for (int yyy=0; yyy<240; yyy++) {
		if (first) {
			first = false;
			vga::SelectPlanes(0xf);
			for (int xxx=0; xxx<80; xxx++) {
				rowPtr[xxx] = 0; }}
		else {
			vga::SelectPlanes(0xf);
			vga::SetBitMask(0x00);  // latches will write
			for (int xxx=0; xxx<80; xxx++) {
				volatile char latchload = prevPtr[xxx];
				rowPtr[xxx] = 0; }
			vga::SetBitMask(0xff); }  // normal write

		int pos;
		switch (patternNum%4) {
		case 0:
			pos = std::sin((T*2.19343) + yyy*(std::sin(T/4.0f)*0.05) * 3.14159 * 2.0) * (std::sin(T*1.781234)*150) + 160;
			break;
		case 1:
			pos = std::sin((T) + yyy*0.005 * 3.14159 * 2.0) * (std::sin(T*3.781234)*150) + 160;
			break;
		case 2:
			pos = std::sin((T*5.666) + yyy*0.008 * 3.14159 * 2.0) * (std::sin(T*1.781234+(yyy*0.010))*50+100) + std::sin((yyy+(T*60))*0.00898)*100+160;
			break;
		case 3:
			pos = std::sin((T*2.45) + yyy*0.012 * 3.14159 * 2.0) * (std::sin(T*1.781234+(yyy*0.010))*66+33) + std::sin((yyy+(T*60))*0.01111)*100+50;
			break; }

		// if (yyy%2==0)
		for (int wx=-4; wx<=4; wx++) {
			PutPixelSlow(pos+wx, yyy, colorpos+wx, dst.addr); }

		prevPtr = rowPtr;
		rowPtr += 80; }}


}  // namespace app
}  // namespace rqdq
