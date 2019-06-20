#include "mem.hpp"

#include <cstdlib>
#include <iostream>
#include <i86.h>  // int386

using std::uint16_t;
using std::uint32_t;

namespace rqdq {
namespace sys {

RealMem::RealMem(uint16_t sizeInBytes) {
	union REGS regs;
	regs.x.eax = 0x0100;  // DPMI: allocate DOS memory
	regs.x.ebx = (sizeInBytes + 15) / 16;  // size in paragraphs
	int386(0x31, &regs, &regs);
	if (regs.w.cflag & 1 != 0) {
		//throw std::runtime_error("no real mem"); }
		std::exit(1); }

	selector_ = regs.w.dx;
	segment_ = regs.w.ax; }


RealMem::~RealMem() {
	if (selector_ != 0) {
		union REGS regs;
		regs.x.eax = 0x101;  // DPMI: free DOS memory
		regs.x.ebx = selector_;
		selector_ = 0;
		segment_ = 0;
		int386(0x31, &regs, &regs); }}


}  // namespace sys
}  // namespace rqdq
