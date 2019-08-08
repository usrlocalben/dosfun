#include "canvas.hpp"

#include "picopng.hpp"
#include "vec.hpp"
#include "vga_mode.hpp"
#include "vga_reg.hpp"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <unordered_map>

namespace rqdq {
namespace rgl {

std::array<std::uint16_t, 32*32> tileDepth;
std::array<std::uint8_t, 32*32> tileColor;

TrueColorCanvas LoadPNG(const std::string& name) {
	std::vector<uint8_t> data;
	picopng::LoadFile(data, name);

	std::vector<uint8_t> buf;
	uint32_t widthInPx;
	uint32_t heightInPx;
	picopng::Decode(buf, widthInPx, heightInPx, data.data(), data.size());

	TrueColorCanvas canvas;
	canvas.Resize({ int(widthInPx), int(heightInPx) });
	std::memcpy(canvas.buf.data(), buf.data(), widthInPx*heightInPx*4);
	return canvas; }


TrueColorCanvas LoadPNG(const uint8_t* data, int cnt) {
	std::vector<uint8_t> buf;
	uint32_t widthInPx;
	uint32_t heightInPx;
	picopng::Decode(buf, widthInPx, heightInPx, data, cnt);

	TrueColorCanvas canvas;
	canvas.Resize({ int(widthInPx), int(heightInPx) });
	std::memcpy(canvas.buf.data(), buf.data(), widthInPx*heightInPx*4);
	return canvas; }


void Resample(TrueColorCanvas& src, TrueColorCanvas& dst) {
	for (int y=0; y<dst.dim.y; y++) {
		for (int x=0; x<dst.dim.x; x++) {
			auto dstCoord = rml::IVec2{ x, y, };
			auto uv = (itof(dstCoord) + 0.5f) / dst.dim;
			auto srcCoord = ftoi(uv * src.dim);
			dst.at(dstCoord) = src.at(srcCoord); }}}


void Convert(const TrueColorCanvas& src, const std::vector<rml::Vec3>& pal, IndexCanvas& dst) {
	assert(src.dim == dst.dim);
	std::unordered_map<uint32_t, int> tab;

	const int width = src.dim.x;
	const int height = src.dim.y;
	const int stride = src.dim.x;

	for (int y=0; y<height; y++) {
		for (int x=0; x<width; x++) {
			auto coord = rml::IVec2{ x, y };
			auto c = src.at(coord);

			uint8_t idx;
			if (auto cached = tab.find(c.to_ulong()); cached != end(tab)) {
				idx = cached->second; }
			else {
				float minDist = 999999.0f;
				int minIdx = -1;
				for (int pi=16; pi<pal.size(); pi++) {
					if (float dist = Length(pal[pi] - ToLinear(c.to_vec3())); dist < minDist) {
						minIdx = pi;
						minDist = dist; }}
				assert(0 <= minIdx && minIdx < pal.size());
				tab[c.to_ulong()] = minIdx;
				idx = minIdx; }

			dst.at(coord) = idx; }}}


/**
 * make a Quake-style color/brightness LUT
 */
std::vector<uint8_t> MakeIndexedBrightnessTable(const std::vector<rml::Vec3>& pal) {
	const auto findNearestColorTo = [&](rml::Vec3 c) {
		float minDist = 9999999.0f;
		int minIdx = -1;
		for (int i=0; i<pal.size(); i++) {
			if (auto dist = Length(pal[i] - c); dist < minDist) {
				minDist = dist;
				minIdx = i; }}
		assert(minIdx >= 0);
		return minIdx; };

	const int numShades = 64;
	std::vector<uint8_t> out(numShades * pal.size());

	for (int ci=0; ci<pal.size(); ci++) {
		for (int si=0; si<numShades; si++) {
			float level = numShades - si - 1;
			float gain = level / (numShades/2);
			auto shadedColor = pal[ci] * gain;
			int idx = findNearestColorTo(shadedColor);
			out[si*pal.size() + ci] = idx; }}

	return out; }


void Copy(const IndexCanvas& src, IndexCanvas& dst, rml::IVec2 origin) {
	for (int y=0; y<dst.dim.y; y++) {
		for (int x=0; x<dst.dim.x; x++) {
			const auto dstCoord = rml::IVec2{ x, y };
			const auto srcCoord = dstCoord + origin;
			dst.at(dstCoord) = src.at(srcCoord); }}}


void PlanarizeLines(IndexCanvas& c) {
	static std::vector<uint8_t> tmp;
	tmp.resize(c.dim.x);
	int span = c.dim.x / 4;
	for (int y=0; y<c.dim.y; y++) {
		for (int x=0; x<c.dim.x; x++) {
			const auto px = c.at({ x, y });
			int pl = x%4;
			int of = x/4;
			tmp[pl*span+of] = px; }
		for (int x=0; x<c.dim.x; x++) {
			c.at({ x, y }) = tmp[x]; }}}


/**
 * convert an indexed-color PNG from picopng's truecolor
 * back into indexed color with a palette
 */
std::pair<IndexCanvas, std::vector<rml::IVec3>> Reindex(const TrueColorCanvas& src) {
	IndexCanvas out;
	out.Resize(src.dim);

	// build table of unique colors and assign indices
	int seq = 0;
	std::unordered_map<uint32_t, int> tab;
	for (int i=0; i<src.buf.size(); i++) {
		auto px = src.buf[i];

		// VGAize the color
		px.r >>= 2;
		px.g >>= 2;
		px.b >>= 2;
		px.a = 0;

		int idx;
		if (auto found = tab.find(px.to_ulong()); found == tab.end()) {
			idx = seq++;
			tab[px.to_ulong()] = idx; }
		else {
			idx = found->second; }
		out.buf[i] = idx; }

	assert(seq <= 256);

	// build VGA palette
	std::vector<rml::IVec3> newPal(256);
	for (const auto& item : tab) {
		const auto& idx = item.second;
		const auto& color = item.first;
		newPal[idx] = rml::IVec3::from_ulong(color); }

	return { out, newPal }; }


/**
 * copy a 32x32 tile of pixels to an unchained screen buffer
 */
void StoreTile(const rml::IVec2 tileOrigin, const std::uint8_t* src, std::uint8_t* dst) {
	assert(tileOrigin.y%32==0 && tileOrigin.x%32==0);
	auto dstLeft = dst + tileOrigin.y*320 + tileOrigin.x/4;
	for (int y=0; y<32; ++y) {
		auto dst32 = reinterpret_cast<uint32_t*>(dstLeft);
		for (int x=0; x<32; x+=16) {
			dst32[ 0] = src[0] | src[4]<<8 | src[ 8]<<16 | src[12]<<24;
			dst32[20] = src[1] | src[5]<<8 | src[ 9]<<16 | src[13]<<24;
			dst32[40] = src[2] | src[6]<<8 | src[10]<<16 | src[14]<<24;
			dst32[60] = src[3] | src[7]<<8 | src[11]<<16 | src[15]<<24;
			dst32 += 1;
			src += 16; }
		dstLeft += 320; }}


/**
 * copy a 32x32 tile of pixels to unchained vga vram
 */
void StoreTile(const rml::IVec2 tileOrigin, const std::uint8_t* src, const vga::VRAMPage& page) {
	assert(tileOrigin.y%32==0 && tileOrigin.x%32==0);
	auto dstTopLeft = page.addr + tileOrigin.y*80 + tileOrigin.x/4;
	auto srcTop = src;
	for (int p=0; p<4; p++) {
		vga::Planes(1<<p);
		auto dstLeft = dstTopLeft;
		auto srcx = src;
		for (int y=0; y<32; y++) {
			auto dst32 = reinterpret_cast<uint32_t*>(dstLeft);
			*dst32 = srcx[12]<<24 | srcx[ 8]<<16 | srcx[ 4]<<8 | srcx[ 0];  dst32++;
			*dst32 = srcx[28]<<24 | srcx[24]<<16 | srcx[20]<<8 | srcx[16];
			srcx += 32;
			dstLeft += 80; }
		src++; }}


}  // namespace rgl
}  // namespace rqdq
