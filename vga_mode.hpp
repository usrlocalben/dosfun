#pragma once
#include "log.hpp"
#include "vga_bios.hpp"
#include "vga_reg.hpp"

#include <cstdint>
#include <cstdlib>

namespace rqdq {
namespace vga {

constexpr int VM_TEXT = 0x03;
constexpr int VM_MODE13 = 0x13;
constexpr int VM_MODEX = 0x100;


struct VRAMPage {
	const int num;
	std::uint8_t* const addr;
	const std::uint16_t vgaAddr; };


const vga::VRAMPage modeXPages[2] = {
	{ 0, vga::VRAM_ADDR, 0 },
	{ 1, vga::VRAM_ADDR + (320*240/4), 320*240/4 } };


void ModeX();


class ModeSetter {
	const int oldMode_;
	int curMode_;

public:
	ModeSetter() :
		oldMode_(bios::Mode()),
		curMode_(oldMode_) {}

	void Set(int req) {
		if (req < 256) {
			bios::Mode(req);
			log::info("vga: set BIOS mode %02x", req);
			curMode_ = req; }
		else if (req == VM_MODEX) {
			vga::ModeX();
			log::info("vga: set Mode X");
			curMode_ = VM_MODEX; }
		else {
			// xxx throw std::runtime_error("unsupported vga mode");
			std::exit(1); }}

	~ModeSetter() {
		if (curMode_ != oldMode_) {
			log::info("vga: restoring prior mode %02x", oldMode_);
			Set(oldMode_); }}};


}  // namespace vga
}  // namespace rqdq
