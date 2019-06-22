#include "pic.hpp"

namespace rqdq {
namespace pic {

IRQLine::IRQLine(int irqNum) :irqNum_(irqNum) {
	if (irqNum < 8) {
		controllerNum_ = 1;
		rotatePort_ = 0x20;
		maskPort_ = 0x21;
		isrNum_ = 0x08 + irqNum; }
	else {
		controllerNum_ = 2;
		rotatePort_ = 0xa0;
		maskPort_ = 0x21;
		isrNum_ = 0x70 + irqNum - 8; }
	stopMask_ = 1 << (irqNum % 8);
	startMask_ = ~stopMask_; }


}  // namespace pic
}  // namespace rqdq
