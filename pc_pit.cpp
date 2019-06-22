#include "pc_pit.hpp"

#include "pc_pic.hpp"

namespace rqdq {
namespace pc {

IRQLine& GetPITIRQLine() {
	static IRQLine irqLine(0);
	return irqLine; }


}  // namespace pc
}  // namespace rqdq
