#pragma once
#include <cstdlib>
#include <cstdint>

#include "vga_reg.hpp"

using std::uint8_t;
using std::uint16_t;

namespace rqdq {
namespace vga {

typedef void (*vbifunc)();

void InstallVBI(vbifunc proc);
void UninstallVBI();

float GetLastVBIFrequency();


class SoftVBI {
public:
	SoftVBI(vbifunc proc) {
		InstallVBI(proc); }
	~SoftVBI() {
		UninstallVBI(); }
	float GetFrequency() {
		return GetLastVBIFrequency(); }};


}  // namespace vga
}  // namespace rqdq
