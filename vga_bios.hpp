#pragma once
#include "pixel.hpp"

#include <array>
#include <cstdint>

namespace rqdq {
namespace vga {

extern const std::array<rgl::TrueColorPixel, 256> kBIOSPalette;

struct BIOSUtil {

	static
	void Mode(std::uint8_t modeNum);

	static
	auto Mode() -> std::uint8_t;

	static
	void Border(std::uint8_t idx); };


}  // close package namespace
}  // close enterprise namespace
