#include "fli.hpp"

#include "vga.hpp"

namespace rqdq {
namespace vga {

volatile int timeInFrames = 0;
volatile int backPage = 1;
volatile bool backLocked = true;

void vbi() {
	++timeInFrames;
	if (!backLocked) {
		vga::SetStartAddress(vga::modeXPages[backPage].vgaAddr);
		backPage ^= 1;
		backLocked = true; }}


}  // namespace vga
}  // namespace rqdq
