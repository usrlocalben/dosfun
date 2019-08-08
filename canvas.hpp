#pragma once
#include "vec.hpp"
#include "vga_mode.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace rqdq {
namespace rgl {

using IndexPixel = std::uint8_t;


struct TrueColorPixel {
	uint8_t r, g, b, a;

	inline uint32_t to_ulong() const {
		return a<<24 | b<<16 | g<<8 | r; }

	inline rml::Vec3 to_vec3() const {
		return { r/255.0f, g/255.0f, b/255.0f }; } };


template<typename T>
struct Canvas {
	std::vector<T> buf;
	int stride;
	rml::IVec2 dim;

	inline T& at(rml::IVec2 coord) {
		return buf[coord.y * stride + coord.x]; }

	inline const T& at(rml::IVec2 coord) const {
		return buf[coord.y * stride + coord.x]; }

	void Resize(rml::IVec2 dim_) {
		dim = dim_;
		buf.resize(dim.x*dim.y);
		stride = dim.x; } };


using TrueColorCanvas = Canvas<TrueColorPixel>;
using IndexCanvas = Canvas<IndexPixel>;

inline rml::Vec3 ToLinear(rml::Vec3 c) {
	return { std::pow(c.x, 2.2f), std::pow(c.y, 2.2f), std::pow(c.z, 2.2f) }; }


inline rml::Vec3 ToSRGB(rml::Vec3 c) {
	return { std::sqrt(c.x), std::sqrt(c.y), std::sqrt(c.z) }; }


TrueColorCanvas LoadPNG(const std::string& name);
TrueColorCanvas LoadPNG(const uint8_t* data, int cnt);

template<typename ARR>
TrueColorCanvas LoadPNG(const ARR& arr) {
	return LoadPNG(arr.data(), arr.size()); }

void Resample(TrueColorCanvas& src, TrueColorCanvas& dst);

void Convert(const TrueColorCanvas& src, const std::vector<rml::Vec3>& pal, IndexCanvas& dst);

std::vector<uint8_t> MakeIndexedBrightnessTable(const std::vector<rml::Vec3>& pal);

void Copy(const IndexCanvas& src, IndexCanvas& dst, rml::IVec2 origin={ 0, 0 });

void PlanarizeLines(IndexCanvas&);

std::pair<IndexCanvas, std::vector<rml::IVec3>> Reindex(const TrueColorCanvas& src);

void StoreTile(const rml::IVec2 tileOrigin, const std::uint8_t* src, std::uint8_t* dst);
void StoreTile(const rml::IVec2 tileOrigin, const std::uint8_t* src, const vga::VRAMPage& page);

extern std::array<std::uint16_t, 32*32> tileDepth;
extern std::array<std::uint8_t, 32*32> tileColor;

}  // namespace rgl
}  // namespace rqdq
