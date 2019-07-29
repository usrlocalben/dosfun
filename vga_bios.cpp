#include "vga_bios.hpp"

#include <cstdint>

#include <dpmi.h>

using std::uint8_t;

namespace rqdq {
namespace vga {

namespace bios {

void SetMode(uint8_t modeNum) {
	__dpmi_regs r;
	r.h.ah = 0;  // set video mode
	r.h.al = modeNum;
	__dpmi_int(0x10, &r); }


std::uint8_t GetMode() {
	__dpmi_regs r;
	r.h.ah = 0x0f;  // get current video mode
	__dpmi_int(0x10, &r);
	return r.h.al; }


}  // namespace bios


}  // namespace vga
}  // namespace rqdq
