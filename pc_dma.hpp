/*
 * support for Intel 8237 DMA controllers
 */
#pragma once
#include <cstdint>
#include <cstring>  // memset
#include <sys/nearptr.h>

#include "os_realmem.hpp"

namespace rqdq {
namespace pc {

class DMABuffer {
public:
	os::RealMem realMem_;
	std::uint32_t addr_;
	int sizeInWords_;

	DMABuffer() :realMem_(), addr_(0), sizeInWords_(0) {}
	DMABuffer(std::uint16_t sizeInWords);
	DMABuffer(const DMABuffer& other) = delete;             // not-copyable
	DMABuffer& operator=(const DMABuffer& other) = delete;  // not-copyable

public:
	/*void Swap(DMABuffer& other) {
		realMem_.Swap(other.realMem_);
		std::swap(addr_, other.addr_);
		std::swap(sizeInWords_, other.sizeInWords_); }*/

	std::uint8_t* Ptr() const {
		return (std::uint8_t*)addr_; }

	std::uint16_t* Ptr16() const {
		return (std::uint16_t*)addr_; }

	std::uint16_t Page() const {
		return addr_ >> 16; }

	std::uint16_t Offset16() const {
		return (addr_ >> 1) % 65536; }

	void Zero() const {
		std::memset(Ptr() + __djgpp_conventional_base, 0, sizeInWords_*2); } };


class DMAChannel {
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
	void Start() const;

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
	const int mode_;  };


}  // namespace pc
}  // namespace rqdq
