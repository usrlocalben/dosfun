#pragma once
#include <cstdint>

namespace rqdq {
namespace vga {

namespace bios {

void SetMode(std::uint8_t modeNum);

std::uint8_t GetMode();


}  // namespace bios


}  // namespace vga
}  // namespace rqdq
