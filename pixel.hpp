#pragma once
#include "vec.hpp"
#include "ryg.hpp"

#include <cstdint>

namespace rqdq {
namespace rgl {

using IndexPixel = std::uint8_t;


struct TrueColorPixel {
	union {
		struct { uint8_t r, g, b, a; };
		uint32_t ui; };

	TrueColorPixel() = default;
	TrueColorPixel(uint32_t u) : ui(u) {}
	TrueColorPixel(uint8_t rr, uint8_t gg, uint8_t bb) :r(rr), g(gg), b(bb) {}

	inline
	auto Vec3() const -> rml::Vec3 {
		using ryg::Linear;
		return { Linear(r), Linear(g), Linear(b) }; }

	inline
	auto bgr() const -> TrueColorPixel {
		return TrueColorPixel(b, g, r); } };


inline
auto sRGB(rml::Vec3 c) -> TrueColorPixel {
	TrueColorPixel px;
	px.r = ryg::sRGB(c.x);
	px.g = ryg::sRGB(c.y);
	px.b = ryg::sRGB(c.z);
	return px; }


}  // namespace rgl
}  // namespace rqdq
