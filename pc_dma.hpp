/*
 * support for Intel 8237 DMA controllers
 */
#pragma once
#include "os_realmem.hpp"

#include <cstdint>
#include <cstring>  // memset

#include <sys/nearptr.h>

namespace rqdq {
namespace pc {

class DMABuffer {

	os::RealMem realMem_;
	std::uint32_t addr_;
	int sizeInWords_;

public:
	DMABuffer() :realMem_(), addr_(0), sizeInWords_(0) {}
	DMABuffer(std::uint16_t sizeInWords);
	// not-copyable
	DMABuffer(const DMABuffer& other) = delete;
	auto operator=(const DMABuffer& other) -> DMABuffer& = delete;

	auto Ptr() const -> std::uint8_t* {
		return (std::uint8_t*)addr_; }

	auto Ptr16() const -> std::uint16_t* {
		return (std::uint16_t*)addr_; }

	auto Page() const -> std::uint16_t {
		return addr_ >> 16; }

	auto Offset8() const -> std::uint16_t {
		return addr_ % 65536; }

	auto Offset16() const -> std::uint16_t {
		return (addr_ >> 1) % 65536; }

	auto Size8() const -> int {
		return sizeInWords_ * 2; }

	auto Size16() const -> int {
		return sizeInWords_; }

	void Zero() const {
		std::memset(Ptr() + __djgpp_conventional_base, 0, sizeInWords_*2); } };


class DMAChannel {
	const int controllerNum_;
	const int channelNum_;
	const int ioBase_;
	const int stride_;

	const int maskPort_;
	const int modePort_;
	const int clearPtrPort_;
	const int baseAddrPort_;
	const int countPort_;
	const int pagePort_;

	const int stopMask_;
	const int startMask_;
	const int mode_;

public:
	DMAChannel(int dmaChannelNum);
	~DMAChannel();
	void Setup(const DMABuffer& buf) const;

	void Stop() const;
private:
	void ClearFlipFlop() const;
	void SetMode() const;
	void SetMemoryAddr(const DMABuffer& buf) const;
	void SetMemorySize(const DMABuffer& buf) const;
	void Start() const; };


}  // namespace pc
}  // namespace rqdq
