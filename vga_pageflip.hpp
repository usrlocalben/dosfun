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
public:
	AnimationPage() {
		locked_ = backLocked; }
private:
	AnimationPage(const AnimationPage&);             // not copyable
	AnimationPage& operator=(const AnimationPage&);  // not copyable

public:
	const VRAMPage& Get() const {
		return modeXPages[backPage]; }

	bool IsLocked() const {
		return locked_; }

	void Unlock() {
		if (locked_) {
			StartAddr(Get().vgaAddr);
			locked_ = false;
			backLocked = false; }}

	~AnimationPage() {
		Unlock(); }
private:
	bool locked_; };


inline int GetTime() {
	return timeInFrames; }


}  // namespace vga
}  // namespace rqdq
