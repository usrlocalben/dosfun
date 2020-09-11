#pragma once
#include <cassert>
#include <cstdint>

namespace rqdq {
namespace alg {

template<int CAP>
class RingIndex {
public:
	static
	std::uint32_t Mod(std::uint32_t x) {
		return x & (CAP-1); }

private:
	std::uint32_t front_{0}, back_{0};

public:
	RingIndex() = default;

	auto Empty() const -> bool {
		return front_ == back_; }

	auto Size() const -> int {
		return static_cast<int>(back_ - front_); }

	auto Available() const -> int {
		return static_cast<int>(CAP - Size()); }

	constexpr
	auto Capacity() const -> int {
		return CAP; }

	auto Loaded() const -> bool {
		return Size() > 0; }

	auto Full() const -> bool {
		return Size() == CAP; }

	auto BackIdx() const -> int {
		return Mod(back_); }

	void PushBack() {
		back_++; }

	auto FrontIdx() const -> int {
		return Mod(front_); }

	void PopFront() {
		assert(Size() > 0);
		front_++; }

	void PopFront(int n) {
		assert(Size() >= n);
		front_ += n; }};


}  // namespace alg
}  // namespace rqdq
