#pragma once
#include <cstdint>
#include <conio.h>  // inp/outp
#include "mem.hpp"

namespace rqdq {

struct DMAPtr {
	RealPtr realPtr;
	std::uint32_t addr;
	int sizeInWords;

	std::uint8_t* Ptr() const {
		return (std::uint8_t*)addr; }

	std::uint16_t* Ptr16() const {
		return (std::uint16_t*)addr; }

	std::uint16_t Page() const {
		return addr >> 16; }

	std::uint16_t Offset16() const {
		return (addr >> 1) % 65536; } };


struct DMAInfo {
	int maskPort;
	int clearPtrPort;
	int modePort;
	int baseAddrPort;
	int countPort;
	int pagePort;
	int stopMask;
	int startMask;
	int mode; };


DMAPtr AllocDMABuffer(std::uint16_t sizeInBytes);
void FreeDMABuffer(DMAPtr ptr);
DMAInfo make_dmainfo(int dmaChannelNum);
void ConfigureTransfer(DMAInfo di, DMAPtr mem);
void StopDMA(DMAInfo di);


}  // namespace rqdq
