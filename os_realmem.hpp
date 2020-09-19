#pragma once
#include <algorithm>
#include <cstdint>

#include <dpmi.h>

namespace rqdq {
namespace os {

class RealMem {
	_go32_dpmi_seginfo info_;

public:
	RealMem() {
		info_.pm_selector = 0; }
	RealMem(std::uint16_t sizeInBytes);
	// not copyable
	auto operator=(const RealMem& other) -> RealMem& = delete;
	RealMem(const RealMem& other) = delete;
	// movable
	auto operator=(RealMem&& other) -> RealMem& {
		std::swap(other.info_, info_);
		return *this; }
	RealMem(RealMem&& other) {
		info_.pm_selector = 0;
		std::swap(other.info_, info_); }
	~RealMem() {
		if (info_.pm_selector != 0) {
			_go32_dpmi_free_dos_memory(&info_);
			info_.pm_selector = 0; }}

	auto Addr() const -> std::uint32_t {
		return info_.rm_segment * 16; }};


}  // namespace os
}  // namespace rqdq
