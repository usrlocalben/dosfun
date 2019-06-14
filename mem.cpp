#include "mem.hpp"

#include <cstdlib>
#include <iostream>
#include <i86.h>  // int386

using std::uint16_t;
using std::uint32_t;

namespace rqdq {

RealPtr AllocReal(uint16_t sizeInBytes) {
	union REGS regs;
	regs.x.eax = 0x0100;  // DPMI: allocate DOS memory
	regs.x.ebx = (sizeInBytes + 15) / 16;  // size in paragraphs
	int386(0x31, &regs, &regs);
	if (regs.w.cflag & 1 != 0) {
		//throw std::runtime_error("no real mem"); }
		std::exit(1); }

	RealPtr out;
	out.selector = regs.w.dx;
	out.segment = regs.w.ax;
	// std::cout << "AllocReal: " << sizeInBytes << " @ 0x" << std::hex << ((uint32_t)out.segment*16) << std::dec << "\n";
	return out; }


void FreeReal(RealPtr ptr) {
	union REGS regs;
	regs.x.eax = 0x101;  // DPMI: free DOS memory
	regs.x.ebx = ptr.selector;
	int386(0x31, &regs, &regs); }


}  // namespace rqdq
