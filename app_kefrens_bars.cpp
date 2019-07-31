#include "app_kefrens_bars.hpp"

#include "amy.hpp"
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


}  // namespace
namespace app {

KefrensBars::KefrensBars() {
	if (false) {
		std::array<int, 8> v8 = { { 0x00, 0x24, 0x49, 0x6d, 0x92, 0xb6, 0xdb, 0xff } };
		std::array<int, 4> v4 = { {    0x00,       0x55,       0xaa,       0xff    } };
		for (int i=0; i<256; i++) {
			int ri = i>>5;
			int gi = (i>>2) & 7;
			int bi = i & 3;
			int rv = v8[ri]>>2;
			int gv = v8[gi]>>2;
			int bv = v4[bi]>>2;
			vga::Color(i, { rv, gv, bv }); }}

	//rgl::TrueColorCanvas tmp = rgl::LoadPNG(rqdq::amyData, 19592);

	std::vector<rml::IVec3> newPal;
	std::tie(bkg_, newPal) = Reindex(rgl::LoadPNG(rqdq::amyData));

	for (int i=0; i<256; i++) {
		vga::Color(i, { newPal[i].x, newPal[i].y, newPal[i].z }); }

	// rgl::TrueColorCanvas bkg = tmp;

	/*
	std::vector<rml::Vec3> vgaPal;
	vgaPal.resize(256);
	for (int i=0; i<256; i++) {
		vgaPal[i] = rgl::ToLinear(vga::ToFloat(vga::Color(i))); }
	*/

	// colorMap_ = rgl::MakeIndexedBrightnessTable(vgaPal);

	/*
	rgl::IndexCanvas tmp2;
	tmp2.Resize(bkg.dim);
	rgl::Convert(bkg, vgaPal, tmp2);
	*/

	// bkg_.Resize({ 320, 240 });
	// rgl::Copy(tmp2, bkg_, { 0, 0 });
	rgl::PlanarizeLines(bkg_); }


void KefrensBars::Draw(const vga::VRAMPage dst, float T, int patternNum, int rowNum) {
	int whole = T;
	float frac = T - whole;

	const int goodLookingColorMagic[] = {
		0, 1, 2, 3, 4,
		6, 15, 18, 19, 21,
		24, 30, 33, 34, 36,
		39, 49, 51, 52 };

	std::array<uint8_t, 320> rowData, rowMask;
	//rowData.fill(0);
	rowMask.fill(0);

	// int magicIdx = patternNum<<1 | (rowNum>>4&1);
	int magicIdx = patternNum;
	uint8_t colorpos = goodLookingColorMagic[magicIdx%19] * 17;
#define SIN std::sin

	uint8_t* rowPtr = dst.addr + __djgpp_conventional_base;
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
				rowData[plane*80+ofs] = colorpos+wx;
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

	if(false) for (int xxx=0; xxx<256; xxx++) {
		int p = xxx%4;
		vga::Planes(1<<p);
		int ofs = xxx/4;
		uint8_t* rowPtr = dst.addr + __djgpp_conventional_base;
		for (int yyy=0; yyy<64; yyy++) {
			rowPtr[ofs] = colorMap_[yyy*256+xxx];
			rowPtr += 80; }}}


}  // namespace app
}  // namespace rqdq
