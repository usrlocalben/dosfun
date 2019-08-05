#pragma once
#include <cstdint>
#include <limits>

#include "kb_tinymod.hpp"

namespace rqdq {
namespace app {

template<int CAP>
class RingIndex {
public:
	RingIndex() :front_(0), back_(0) {}

	static std::uint32_t Mod(std::uint32_t x) {
		return x & (CAP-1); }

	bool Empty() const {
		return front_ == back_; }

	std::uint32_t Size() const {
		return back_ - front_; }

	std::uint32_t Available() const {
		return CAP - Size(); }

	const int Capacity() const {
		return CAP; }

	bool Full() const { return Size() == CAP; }

	int BackIdx() const {
		return Mod(back_); }

	void PushBack() {
		back_++; }

	int FrontIdx() const {
		return Mod(front_); }

	void PopFront() {
		front_++; }

private:
	std::uint32_t front_;
	std::uint32_t back_; };


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
	RingIndex<4096> rw_;
	std::int32_t buf_[4096*2];
	int pbuf_[4096*2]; };


}  // namespace app
}  // namespace rqdq
