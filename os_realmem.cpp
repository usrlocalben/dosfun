#include "os_realmem.hpp"

#include <cstdlib>

#include <dpmi.h>

using std::uint16_t;
using std::uint32_t;

namespace rqdq {
namespace os {

RealMem::RealMem(uint16_t sizeInBytes) {
	info_.size = (sizeInBytes + 15) / 16;  // size in paragraphs
	int result = _go32_dpmi_allocate_dos_memory(&info_);
	if (result != 0) {
		//throw std::runtime_error("no real mem"); }
		std::exit(1); }}


}  // namespace os
}  // namespace rqdq
