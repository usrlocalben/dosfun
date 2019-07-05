#pragma once
#include <algorithm>
#include <cstdint>
#include <dpmi.h>

namespace rqdq {
namespace os {

class RealMem {
public:
	RealMem() {
		info_.pm_selector = 0; }
	RealMem(std::uint16_t sizeInBytes);
	RealMem& operator=(const RealMem& other) = delete;
	RealMem(const RealMem& other) = delete;
	RealMem& operator=(RealMem&& other) {
		std::swap(other.info_, info_);
		return *this; }
	RealMem(RealMem&& other) {
		info_.pm_selector = 0;
		std::swap(other.info_, info_); }
	~RealMem() {
		if (info_.pm_selector != 0) {
			_go32_dpmi_free_dos_memory(&info_);
			info_.pm_selector = 0; }}

	uint32_t GetAddr() const {
		return info_.rm_segment * 16; }

private:
	_go32_dpmi_seginfo info_; };


}  // namespace os
}  // namespace rqdq
