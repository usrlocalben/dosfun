#include "canvas.hpp"

#include "picopng.hpp"
#include "vec.hpp"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <unordered_map>

namespace rqdq {
namespace rgl {

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


}  // namespace rgl
}  // namespace rqdq
