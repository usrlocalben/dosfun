#pragma once
#include "alg_ringindex.hpp"
#include "kb_tinymod.hpp"

#include <cstdint>

namespace rqdq {
namespace app {

class PlayerAdapter {
public:
	constexpr static int capacity = 4096;

	static
	void BlasterJmp(void* out, int fmt, int numChannels, int numSamples, void* self);

private:
	kb::ModPlayer& player_;
	alg::RingIndex<capacity> rw_;
	std::int32_t buf_[capacity*2];
	int pbuf_[capacity*2];

public:
	PlayerAdapter(kb::ModPlayer& p) :player_(p), rw_() {}

	void Refill();

	auto Full() const -> bool {
		return rw_.Full(); }

	auto Low() const -> bool {
		return rw_.Size() < (rw_.Capacity() / 3); }

private:
	void BlasterProc(void* out, int fmt, int numChannels, int numSamples);

	void PushBack(int l, int r) {
		int idx = rw_.BackIdx();
		buf_[idx] = l;
		buf_[idx+capacity] = r;
		rw_.PushBack(); }

	void PopFront(int& l, int& r) {
		int idx = rw_.FrontIdx();
		l = buf_[idx];
		r = buf_[idx+capacity];
		rw_.PopFront(); }};


}  // namespace app
}  // namespace rqdq
