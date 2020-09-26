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
	std::int16_t buf_[capacity*2];
	std::int16_t pbuf_[capacity*2];

public:
	PlayerAdapter(kb::ModPlayer& p) :player_(p), rw_() {}

	void Refill();

	auto Full() const -> bool {
		return rw_.Full(); }

	auto Low() const -> bool {
		return rw_.Size() < (rw_.Capacity() / 3); }

private:
	void BlasterProc(void* out, int fmt, int numChannels, int numSamples);

	void PushBack(std::int16_t l, std::int16_t r) {
		int idx = rw_.BackIdx();
		buf_[idx*2+0] = l;
		buf_[idx*2+1] = r;
		rw_.PushBack(); }

	void PopFront(std::int16_t& l, std::int16_t& r) {
		int idx = rw_.FrontIdx();
		l = buf_[idx*2+0];
		r = buf_[idx*2+1];
		rw_.PopFront(); }};


}  // namespace app
}  // namespace rqdq
