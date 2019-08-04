#pragma once
#include "algorithm.hpp"
#include "kb_tinymod.hpp"

#include <cstdint>
#include <limits>

namespace rqdq {
namespace app {

class PlayerAdapter {
public:
	PlayerAdapter(kb::ModPlayer& p) :player_(p), rw_() {}

	static void BlasterJmp(void* out, int fmt, int numChannels, int numSamples, void* self);

	void Refill();

	bool Full() const {
		return rw_.Full(); }

	bool Low() const {
		return rw_.Size() < (rw_.Capacity() / 3); }

private:
	void BlasterProc(void* out, int fmt, int numChannels, int numSamples);

	void PushBack(int l, int r) {
		int idx = rw_.BackIdx();
		buf_[idx] = l;
		buf_[idx+4096] = r;
		rw_.PushBack(); }

	void PopFront(int& l, int& r) {
		int idx = rw_.FrontIdx();
		l = buf_[idx];
		r = buf_[idx+4096];
		rw_.PopFront(); }

private:
	kb::ModPlayer& player_;
	alg::RingIndex<4096> rw_;
	std::int32_t buf_[4096*2];
	int pbuf_[4096*2]; };


}  // namespace app
}  // namespace rqdq
