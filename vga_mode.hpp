#pragma once
#include <cstdint>
#include <cstdlib>

#include "vga_reg.hpp"

using std::uint8_t;
using std::uint16_t;

namespace rqdq {
namespace vga {

const int VM_TEXT = 3;
const int VM_MODE13 = 0x13;
const int VM_MODEX = 0x100;


struct VRAMPage {
	const int num;
	std::uint8_t* const addr;
	const std::uint16_t vgaAddr; };


const vga::VRAMPage modeXPages[2] = {
	{ 0, vga::VGAPTR, 0 },
	{ 1, vga::VGAPTR + (320*240/4), 320*240/4 } };


void SetBIOSMode(int num);
void SetModeX();


class ModeSetter {
public:
	// XXX assume text-mode, need detection
	ModeSetter() :oldMode_(0x3), curMode_(oldMode_) {}

	void Set(int req) {
		if (req == VM_TEXT) {
			vga::SetBIOSMode(0x3);
			curMode_ = VM_TEXT; }
		else if (req == VM_MODE13) {
			vga::SetBIOSMode(0x13);
			curMode_ = VM_MODE13; }
		else if (req == VM_MODEX) {
			vga::SetModeX();
			curMode_ = VM_MODEX; }
		else {
			// XXX
			std::exit(1); }}

	~ModeSetter() {
		if (curMode_ != oldMode_) {
			Set(oldMode_); }}

private:
		const int oldMode_;
		int curMode_; };


}  // namespace vga
}  // namespace rqdq
