#include "pc_pic.hpp"

namespace rqdq {
namespace pc {

IRQLineRT::IRQLineRT(int irqNum)
		:irqNum_(irqNum),
		savedISRPtr_(0),
		controllerNum_(irqNum < 8 ? 1 : 2),
		rotatePort_(irqNum < 8 ? 0x20 : 0xa0),
		maskPort_(irqNum < 8 ? 0x21 : 0xa1),
		isrNum_(irqNum < 8 ? 0x08+irqNum : 0x70 + irqNum - 8),
		stopMask_(1 << (irqNum % 8)),
		startMask_(~stopMask_) {}


}  // namespace pc
}  // namespace rqdq
