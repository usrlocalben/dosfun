#pragma once
#include <cstdint>
#include <cstdlib>

#include "vga_bios.hpp"
#include "vga_reg.hpp"

using std::uint8_t;
using std::uint16_t;

namespace rqdq {
namespace vga {

const int VM_TEXT = 0x03;
const int VM_MODE13 = 0x13;
const int VM_MODEX = 0x100;


struct VRAMPage {
	const int num;
	std::uint8_t* const addr;
	const std::uint16_t vgaAddr; };


const vga::VRAMPage modeXPages[2] = {
	{ 0, vga::VGAPTR, 0 },
	{ 1, vga::VGAPTR + (320*240/4), 320*240/4 } };


void SetModeX();


class ModeSetter {
public:
	ModeSetter()
		:oldMode_(bios::GetMode()),
		curMode_(oldMode_) {}

	void Set(int req) {
		if (req < 256) {
			bios::SetMode(req);
			curMode_ = req; }
		else if (req == VM_MODEX) {
			vga::SetModeX();
			curMode_ = VM_MODEX; }
		else {
			// xxx throw std::runtime_error("unsupported vga mode");
			std::exit(1); }}

	~ModeSetter() {
		if (curMode_ != oldMode_) {
			Set(oldMode_); }}

private:
		const int oldMode_;
		int curMode_; };


}  // namespace vga
}  // namespace rqdq
