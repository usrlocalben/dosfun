#pragma once
#include <cstdint>
#include <limits>

#include "kb_tinymod.hpp"

namespace rqdq {
namespace app {

template<int CAP>
class ReaderWriterBufferIdx {
public:
	ReaderWriterBufferIdx() :rPos_(0), wPos_(0) {}

	static std::uint32_t Mod(std::uint32_t x) {
		return x & (CAP-1); }

	bool Empty() {
		return rPos_ == wPos_; }

	std::uint32_t Size() {
		return wPos_ - rPos_; }

	std::uint32_t Available() {
		return CAP - Size(); }

	bool Full() {
		return Size() == CAP; }

	int BeginPush() {
		return Mod(wPos_); }

	void EndPush() {
		wPos_++; }

	int BeginShift() {
		return Mod(rPos_); }

	void EndShift() {
		rPos_++; }

private:
	std::uint32_t rPos_;
	std::uint32_t wPos_; };


class PlayerAdapter {
public:
	PlayerAdapter(kb::ModPlayer& p) :player_(p), rw_() {}

	static void BlasterJmp(void* out, int fmt, int numChannels, int numSamples, void* self);

	void Refill();

private:
	void BlasterProc(void* out, int fmt, int numChannels, int numSamples);

	void Push(int l, int r) {
		int idx = rw_.BeginPush();
		buf_[idx] = l;
		buf_[idx+4096] = r;
		rw_.EndPush(); }

	void Shift(int& l, int& r) {
		int idx = rw_.BeginShift();
		l = buf_[idx];
		r = buf_[idx+4096];
		rw_.EndShift(); }

private:
	kb::ModPlayer& player_;
	ReaderWriterBufferIdx<4096> rw_;
	std::int16_t buf_[4096*2];
	float pbuf_[4096*2]; };


}  // namespace app
}  // namespace rqdq
