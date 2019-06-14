#pragma once
#include <cstdint>

namespace rqdq {

struct RealPtr {
	std::uint16_t selector;
	std::uint16_t segment; };

RealPtr AllocReal(std::uint16_t sizeInBytes);
void FreeReal(RealPtr ptr);


}  // namespace rqdq
