#pragma once
#include "vec.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace rqdq {
namespace rgl {

struct TrueColorPixel {
	uint8_t r, g, b, a;

	uint32_t to_ulong() const {
		return a<<24 | b<<16 | g<<8 | r; }

	rml::Vec3 to_vec3() const {
		return { r/255.0f, g/255.0f, b/255.0f }; } };


struct TrueColorCanvas {
	std::vector<TrueColorPixel> buf;
	int stride;
	rml::IVec2 dim;

	TrueColorPixel& at(rml::IVec2 coord) {
		return buf[coord.y * stride + coord.x]; }

	const TrueColorPixel& at(rml::IVec2 coord) const {
		return buf[coord.y * stride + coord.x]; }

	void Resize(rml::IVec2 dim_) {
		dim = dim_;
		buf.resize(dim.x*dim.y);
		stride = dim.x; } };


struct IndexCanvas {
	std::vector<uint8_t> buf;
	int stride;
	rml::IVec2 dim;

	uint8_t& at(rml::IVec2 coord) {
		return buf[coord.y * stride + coord.x]; }

	const uint8_t& at(rml::IVec2 coord) const {
		return buf[coord.y * stride + coord.x]; }

	void Resize(rml::IVec2 dim_) {
		dim = dim_;
		buf.resize(dim.x*dim.y);
		stride = dim.x; } };


inline rml::Vec3 ToLinear(rml::Vec3 c) {
	return { std::pow(c.x, 2.2f), std::pow(c.y, 2.2f), std::pow(c.z, 2.2f) }; }


inline rml::Vec3 ToSRGB(rml::Vec3 c) {
	return { std::sqrt(c.x), std::sqrt(c.y), std::sqrt(c.z) }; }


TrueColorCanvas LoadPNG(const std::string& name);
TrueColorCanvas LoadPNG(const uint8_t* data, int cnt);

void Resample(TrueColorCanvas& src, TrueColorCanvas& dst);

void Convert(const TrueColorCanvas& src, const std::vector<rml::Vec3>& pal, IndexCanvas& dst);

std::vector<uint8_t> MakeIndexedBrightnessTable(const std::vector<rml::Vec3>& pal);

void Copy(const IndexCanvas& src, IndexCanvas& dst, rml::IVec2 origin={ 0, 0 });

void PlanarizeLines(IndexCanvas&);

std::pair<IndexCanvas, std::vector<rml::IVec3>> Reindex(const TrueColorCanvas& src);

}  // namespace rgl
}  // namespace rqdq
