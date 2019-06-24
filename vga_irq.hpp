#pragma once
#include <cstdlib>
#include <cstdint>

#include "vga_reg.hpp"

using std::uint8_t;
using std::uint16_t;

namespace rqdq {
namespace vga {

typedef void (*vbifunc)();


class RetraceIRQ {
public:
	RetraceIRQ(vbifunc proc);
	~RetraceIRQ();
private:
	RetraceIRQ& operator=(const RetraceIRQ&);  // non-copyable
	RetraceIRQ(const RetraceIRQ&);          // non-copyable
public:
	float GetHz() const; };


}  // namespace vga
}  // namespace rqdq
