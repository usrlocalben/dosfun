#pragma once
#include <algorithm>
#include <cstdint>

namespace rqdq {
namespace sys {

class RealMem {
public:
	RealMem() :selector_(0), segment_(0) {}
	RealMem(std::uint16_t sizeInBytes);
	~RealMem();
private:
	RealMem& operator=(const RealMem& other);
	RealMem(const RealMem& other);

public:
	inline void Swap(RealMem& other) {
		std::swap(selector_, other.selector_);
		std::swap(segment_, other.segment_); }

public:
	std::uint16_t selector_;
	std::uint16_t segment_; };


}  // namespace sys
}  // namespace rqdq
