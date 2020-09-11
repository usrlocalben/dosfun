#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace picopng {

extern void LoadFile(std::vector<uint8_t>& buffer, const std::string& filename);

extern int Decode(
	std::vector<uint8_t>& out_image,
	unsigned long& image_width,
	unsigned long& image_height,
	const unsigned char* in_png,
	size_t in_size,
	bool convert_to_rgba32=true
);

}  // namespace picopng
