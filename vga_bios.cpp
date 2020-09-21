#include "vga_bios.hpp"

#include <cstdint>

#include <dpmi.h>

using std::uint8_t;

namespace rqdq {
namespace vga {

namespace bios {

void Mode(uint8_t modeNum) {
	__dpmi_regs r;
	r.h.ah = 0;  // set video mode
	r.h.al = modeNum;
	__dpmi_int(0x10, &r); }


auto Mode() -> std::uint8_t {
	__dpmi_regs r;
	r.h.ah = 0x0f;  // get current video mode
	__dpmi_int(0x10, &r);
	return r.h.al; }


void Border(uint8_t idx) {
	__dpmi_regs r;
	r.x.ax = 0x1001;
	r.h.bh = idx;
	__dpmi_int(0x10, &r); }


}  // namespace bios


}  // namespace vga
}  // namespace rqdq
