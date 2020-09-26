#include "app_kefrens_bars.hpp"

#include "data_amy.hpp"
#include "canvas.hpp"
#include "picopng.hpp"
#include "vec.hpp"
#include "vga_mode.hpp"
#include "vga_reg.hpp"
#include "vga_bios.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <cstdio>

using std::uint8_t, std::uint16_t, std::uint32_t;

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

constexpr int devStride = 80;
constexpr int devWidth = 320;
constexpr int devHeight = 240;

}  // namespace
namespace app {


class KefrensBars::impl {

	rgl::IndexCanvas bkg_;
	std::vector<uint8_t> colorMap_;
	bool paletteFixed_{false};
	std::vector<rgl::TrueColorPixel> newPal_;
	alignas(16) std::uint8_t palBuf_[256*3];

public:
	impl() {
		std::vector<rml::Vec3> biosPal(256);
		for (int i=0; i<256; ++i) {
			biosPal[i] = vga::kBIOSPalette[i].Linear(); }

		std::tie(bkg_, newPal_) = Indexed(rgl::LoadPNG(data::amy));

		//for (int i=0; i<256; i++) {
		//	vga::Color(i, newPal[i]); }

		std::vector<rml::Vec3> tmp(256);
		for (int i=0; i<256; ++i) {
			tmp[i] = newPal_[i].Linear(); }

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
				glcm2[i*16 + c + 4] = NearestColor(biosPal[idx]); }}

		rgl::PlanarizeLines(bkg_); }

	void Draw(vga::DrawContext& dc, float T, int patternNum, int rowNum) {
		int whole = T;
		float frac = T - whole;

		if (!paletteFixed_) {
			constexpr float fadeTimeInSecs = 2.0F;
			if (T > fadeTimeInSecs) {
				dc.Palette(0, 256, newPal_.data());
				paletteFixed_ = true; }
			else {
				// fade (in gamma space), 8-bit fixed point w/10-bit shift for vga 18-bit color
				auto level = static_cast<int>(T / fadeTimeInSecs * 256.0);
				for (int i=0; i<256; ++i) {
					palBuf_[i*3+0] = (static_cast<int32_t>(newPal_[i].r) * level) >> 10;
					palBuf_[i*3+1] = (static_cast<int32_t>(newPal_[i].g) * level) >> 10;
					palBuf_[i*3+2] = (static_cast<int32_t>(newPal_[i].b) * level) >> 10; }
				dc.Palette(0, 256, palBuf_); }}

		std::array<uint8_t, devWidth> rowData, rowMask;
		//rowData.fill(0);
		rowMask.fill(0);

		// int magicIdx = patternNum<<1 | (rowNum>>4&1);
		int magicIdx = patternNum;
		uint8_t colorpos = magicIdx%19;
		alignas(16) uint8_t barImg[16];
		for (int i=0; i<9; ++i) {
			barImg[i] = glcm2[colorpos*16 + i]; }

#define SIN std::sin

		int pageNum = dc.Get();
		int nextStartAddr = pageNum*devHeight*devStride;
		auto destRow = static_cast<uint8_t*>(vga::Map(nextStartAddr));
		for (int yyy=0; yyy<devHeight; yyy++) {
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
				if (0 <= ox && ox < devWidth) {
					int plane = ox&3;
					int ofs = ox>>2;
					rowData[plane*devStride+ofs] = barImg[wx+4];
					rowMask[plane*devStride+ofs] = 0xff; }}

			// copy row[] to vram planes
			for (int p=0; p<4; p++) {
				vga::Planes(1<<p);
				for (int rx=0; rx<devStride; rx+=4) {
					auto bPx = *reinterpret_cast<uint32_t*>(bkg_.buf.data() + (yyy * bkg_.stride) + (p*devStride) + rx);
					auto fPx = *reinterpret_cast<uint32_t*>(rowData.data() + (p*devStride) + rx);
					auto mask = *reinterpret_cast<uint32_t*>(rowMask.data() + (p*devStride) + rx);
					*reinterpret_cast<uint32_t*>(destRow + rx) = Blend(fPx, bPx, mask); }
#ifdef SHOW_TIMING
				destRow[79] = 255;
#endif
				}

			destRow += devStride; }

		dc.StartAddr(nextStartAddr); }};


KefrensBars::KefrensBars() :
	impl_(std::make_unique<impl>()) {}

KefrensBars::~KefrensBars() = default;

void KefrensBars::Draw(vga::DrawContext& dc, float T, int patternNum, int rowNum) {
	return impl_->Draw(dc, T, patternNum, rowNum); }


}  // namespace app
}  // namespace rqdq
