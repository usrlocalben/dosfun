#pragma once
#include <cstdint>

namespace rqdq {
namespace vga {

namespace bios {

void Mode(std::uint8_t modeNum);

auto Mode() -> std::uint8_t;

void Border(std::uint8_t idx);


}  // namespace bios


}  // namespace vga
}  // namespace rqdq
