#pragma once
#include "canvas.hpp"
#include "vga_mode.hpp"

namespace rqdq {
namespace app {

class KefrensBars {
	rgl::IndexCanvas bkg_;
	std::vector<uint8_t> colorMap_;
public:
	KefrensBars();
	void Draw(const vga::VRAMPage dst, float T, int patternNum, int rowNum); };


}  // namespace app
}  // namespace rqdq
