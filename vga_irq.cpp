#include "vga_irq.hpp"

#include "pc_pit.hpp"

namespace rqdq {
namespace vga {

pc::IRQLine& pitIRQLine = pc::GetPITIRQLine();

int irqSleepTimeInTicks = 0;


}  // namespace vga
}  // namespace rqdq
