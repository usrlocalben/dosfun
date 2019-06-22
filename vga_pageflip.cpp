#include "vga_pageflip.hpp"

#include "vga_mode.hpp"
#include "vga_reg.hpp"

namespace rqdq {
namespace vga {

volatile int timeInFrames = 0;
volatile int backPage = 1;
volatile bool backLocked = true;

void vbi() {
	++timeInFrames;
	if (!backLocked) {
		SetStartAddress(modeXPages[backPage].vgaAddr);
		backPage ^= 1;
		backLocked = true; }}


}  // namespace vga
}  // namespace rqdq
