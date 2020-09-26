#pragma once
#include "pixel.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace rqdq {
namespace rgl {

template<typename T>
struct Canvas {
	std::vector<T> buf;
	int stride;
	rml::IVec2 dim;

	inline
	auto at(rml::IVec2 coord) -> T& {
		return buf[coord.y*stride + coord.x]; }

	inline
	auto at(rml::IVec2 coord) const -> const T& {
		return buf[coord.y*stride + coord.x]; }

	void Resize(rml::IVec2 dim_) {
		dim = dim_;
		buf.resize(dim.x*dim.y);
		stride = dim.x; } };


using TrueColorCanvas = Canvas<TrueColorPixel>;
using IndexCanvas = Canvas<IndexPixel>;


auto LoadPNG(const std::string& name) -> TrueColorCanvas;
auto LoadPNG(const uint8_t* data, int cnt) -> TrueColorCanvas;

template<typename ARR>
auto LoadPNG(const ARR& arr) -> TrueColorCanvas {
	return LoadPNG(arr.data(), arr.size()); }

void Resample(TrueColorCanvas& src, TrueColorCanvas& dst);

void Convert(const TrueColorCanvas& src, const std::vector<rml::Vec3>& pal, IndexCanvas& dst);

auto MakeIndexedBrightnessTable(const std::vector<rml::Vec3>& pal) -> std::vector<uint8_t>;

void Copy(const IndexCanvas& src, IndexCanvas& dst, rml::IVec2 origin={ 0, 0 });

void PlanarizeLines(IndexCanvas&);

auto Indexed(const TrueColorCanvas&) -> std::pair<IndexCanvas, std::vector<rgl::TrueColorPixel>>;


}  // namespace rgl
}  // namespace rqdq
