#pragma once
#include "vga_mode.hpp"

namespace rqdq {
namespace vga {

extern volatile int timeInFrames;
extern volatile int backPage;
extern volatile bool backLocked;


struct FlipPages {
	void operator()() {
		++timeInFrames;
		if (!backLocked) {
			backPage ^= 1;
			backLocked = true; }}};


class AnimationPage {

	bool locked_; 

public:
	AnimationPage() :
		locked_(backLocked) {}

	// not copyable
	AnimationPage(const AnimationPage&) = delete;
	auto operator=(const AnimationPage&) -> AnimationPage& = delete;

	~AnimationPage() {
		Unlock(); }

public:
	auto Get() const -> const VRAMPage& {
		return modeXPages[backPage]; }

	auto IsLocked() const -> bool {
		return locked_; }

	void Unlock() {
		if (locked_) {
			StartAddr(Get().vgaAddr);
			locked_ = false;
			backLocked = false; }}};


inline
auto GetTime() -> int {
	return timeInFrames; }


}  // namespace vga
}  // namespace rqdq
