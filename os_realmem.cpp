#include "os_realmem.hpp"

#include <cstdlib>
#include <iostream>
#include <dpmi.h>  // __dpmi_int

using std::uint16_t;
using std::uint32_t;

namespace rqdq {
namespace os {

RealMem::RealMem(uint16_t sizeInBytes) {
	__dpmi_regs regs;
	regs.d.eax = 0x0100;  // DPMI: allocate DOS memory
	regs.d.ebx = (sizeInBytes + 15) / 16;  // size in paragraphs
	__dpmi_int(0x31, &regs);
	if (regs.x.flags & 1 != 0) {
		//throw std::runtime_error("no real mem"); }
		std::exit(1); }

	selector_ = regs.x.dx;
	segment_ = regs.x.ax; }


RealMem::~RealMem() {
	if (selector_ != 0) {
		__dpmi_regs regs;
		regs.d.eax = 0x101;  // DPMI: free DOS memory
		regs.d.ebx = selector_;
		selector_ = 0;
		segment_ = 0;
		__dpmi_int(0x31, &regs); }}


}  // namespace os
}  // namespace rqdq
