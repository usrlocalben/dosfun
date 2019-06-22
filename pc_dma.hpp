#pragma once
#include <cstdint>
#include <cstring>  // memset

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
private:
	DMABuffer(const DMABuffer& other);             // not copyable
	DMABuffer& operator=(const DMABuffer& other);  // not copyable

public:
	void Swap(DMABuffer& other) {
		realMem_.Swap(other.realMem_);
		std::swap(addr_, other.addr_);
		std::swap(sizeInWords_, other.sizeInWords_); }

	std::uint8_t* Ptr() const {
		return (std::uint8_t*)addr_; }

	std::uint16_t* Ptr16() const {
		return (std::uint16_t*)addr_; }

	std::uint16_t Page() const {
		return addr_ >> 16; }

	std::uint16_t Offset16() const {
		return (addr_ >> 1) % 65536; }

	void Zero() const {
		std::memset(Ptr(), 0, sizeInWords_*2); } };


struct Channel {
	int maskPort;
	int clearPtrPort;
	int modePort;
	int baseAddrPort;
	int countPort;
	int pagePort;
	int stopMask;
	int startMask;
	int mode; };


Channel make_channel(int channelNum);
void Configure(const Channel ch, const DMABuffer& buf);
void Stop(Channel ch);


}  // namespace pc
}  // namespace rqdq
