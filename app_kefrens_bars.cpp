#include "app_kefrens_bars.hpp"

#include "bkg.hpp"
#include "canvas.hpp"
#include "picopng.hpp"
#include "vec.hpp"
#include "vga_mode.hpp"
#include "vga_reg.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>

#include <sys/nearptr.h>

using std::uint8_t;

namespace rqdq {
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

	rgl::TrueColorCanvas tmp;
	{
		std::vector<uint8_t> buf;
		uint32_t wpx, hpx;
		picopng::Decode(buf, wpx, hpx, rqdq::bkgData, 142476);
		tmp.Resize({ int(wpx), int(hpx) });
		std::memcpy(tmp.buf.data(), buf.data(), wpx*hpx*4); }

	// auto big = rgl::LoadPNG("bkg.png");
	// assert(big.dim.x > 0);
	// assert(big.dim.y > 0);

	rgl::TrueColorCanvas bkg = tmp;
	//bkg.Resize({ 320, 240 });
	//Resample(tmp, bkg);

	std::vector<rml::Vec3> vgaPal;
	vgaPal.resize(256);
	for (int i=0; i<256; i++) {
		vgaPal[i] = rgl::ToLinear(vga::ToFloat(vga::Color(i))); }

	colorMap_ = rgl::MakeIndexedBrightnessTable(vgaPal);

	bkg_.Resize(bkg.dim);
	rgl::Convert(bkg, vgaPal, bkg_);

	for (int y=0; y<240; y++) {
		char tmp[320];
		for (int x=0; x<320; x++) {
			const auto px = bkg_.at({ x, y });
			int pl = x%4;
			int of = x/4;
			tmp[pl*80+of] = px; }
		for (int x=0; x<320; x++) {
			bkg_.at({ x, y }) = tmp[x]; }}
}


void KefrensBars::Draw(const vga::VRAMPage dst, float T, int patternNum, int rowNum) {
	int whole = T;
	float frac = T - whole;

	const int goodLookingColorMagic[] = {
		0, 1, 2, 3, 4,
		6, 15, 18, 19, 21,
		24, 30, 33, 34, 36,
		39, 49, 51, 52 };

	uint8_t row[320];
	std::memset(row, 0, 320);

	uint8_t rowMask[320];
	std::memset(rowMask, 0, 320);

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
				row[plane*80+ofs] = colorpos+wx;
				rowMask[plane*80+ofs] = 0xff; }}

		// copy row[] to vram planes
		for (int p=0; p<4; p++) {
			vga::Planes(1<<p);
			for (int rx=0; rx<80; rx+=4) {
				uint32_t bPx = *reinterpret_cast<uint32_t*>(bkg_.buf.data() + (yyy * bkg_.stride) + (p*80) + rx);
				uint32_t fPx = *reinterpret_cast<uint32_t*>(row + (p*80) + rx);
				uint32_t mask = *reinterpret_cast<uint32_t*>(rowMask + (p*80) + rx);
				uint32_t merged = (fPx&mask) | (bPx&~mask);
				*reinterpret_cast<uint32_t*>(rowPtr + rx) = merged; }
			rowPtr[79] = 0; }

		rowPtr += 80;
	}

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
