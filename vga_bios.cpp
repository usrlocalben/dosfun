#include "vga_bios.hpp"

#include <cstdint>

#include "i86.h"  // int386

using std::uint8_t;

namespace rqdq {
namespace vga {

namespace bios {

void SetMode(uint8_t modeNum) {
	union REGS r;
	r.h.ah = 0;  // set video mode
	r.h.al = modeNum;
	int386(0x10, &r, &r); }


std::uint8_t GetMode() {
	union REGS r;
	r.h.ah = 0x0f;  // get current video mode
	int386(0x10, &r, &r);
	return r.h.al; }


}  // namespace bios


}  // namespace vga
}  // namespace rqdq
