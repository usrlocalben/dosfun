#include "pic.hpp"

namespace rqdq {
namespace pic {

IRQLine make_irqline(int irqNum) {
	IRQLine out;
	out.irqNum = irqNum;
	if (irqNum < 8) {
		out.controllerNum = 1;
		out.rotatePort = 0x20;
		out.maskPort = 0x21;
		out.isrNum = 0x08 + irqNum; }
	else {
		out.controllerNum = 2;
		out.rotatePort = 0xa0;
		out.maskPort = 0x21;
		out.isrNum = 0x70 + irqNum - 8; }
	out.stopMask = 1 << (irqNum % 8);
	out.startMask = ~out.stopMask;
	return out; }


}  // namespace pic
}  // namespace rqdq
