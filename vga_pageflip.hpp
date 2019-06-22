#pragma once
#include "vga_mode.hpp"

namespace rqdq {
namespace vga {

extern volatile int timeInFrames;
extern volatile int backPage;
extern volatile bool backLocked;

void vbi();

class VRAMLock {
public:
	VRAMLock() {
		locked_ = backLocked; }
	~VRAMLock() {
		if (locked_) {
			backLocked = false; }}
	const VRAMPage& Page() {
		return modeXPages[backPage]; }
	bool IsLocked() {
		return locked_; }
private:
	bool locked_; };


inline int GetTime() {
	return timeInFrames; }


}  // namespace vga
}  // namespace rqdq
