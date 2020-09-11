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
	AnimationPage() {
		locked_ = backLocked; }
private:
	AnimationPage(const AnimationPage&);             // not copyable
	AnimationPage& operator=(const AnimationPage&);  // not copyable

public:
	auto Get() const -> const VRAMPage& {
		return modeXPages[backPage]; }

	auto IsLocked() const -> bool {
		return locked_; }

	void Unlock() {
		if (locked_) {
			StartAddr(Get().vgaAddr);
			locked_ = false;
			backLocked = false; }}

	~AnimationPage() {
		Unlock(); }};


inline
auto GetTime() -> int {
	return timeInFrames; }


}  // namespace vga
}  // namespace rqdq
