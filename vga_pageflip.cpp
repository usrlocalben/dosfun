#include "vga_pageflip.hpp"

#include "vga_mode.hpp"
#include "vga_reg.hpp"

#include <cstdint>

namespace rqdq {
namespace vga {

int timeInFrames{0};
int backPage{1};
bool backLocked{true};
int paletteStart{0};
int paletteCnt{0};
alignas(16) std::uint8_t paletteBuffer[768];
std::uint8_t* paletteData{paletteBuffer};


}  // namespace vga
}  // namespace rqdq
