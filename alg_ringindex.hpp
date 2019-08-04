#pragma once
#include <cstdint>

namespace rqdq {
namespace alg {

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


}  // namespace alg
}  // namespace rqdq
