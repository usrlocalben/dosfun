#pragma once
#include <cstdint>
#include <conio.h>  // inp/outp

namespace rqdq {
namespace pic {

class IRQLine {
public:
	IRQLine(int irqNum);

	inline void SignalEOI() const {
		if (irqNum_ >= 8) {
			outp(0xa0, 0x20); }
		outp(0x20, 0x20); }

	inline bool IsReal() const {
		if (irqNum_ == 7) {
			outp(0x20, 0x0b);  // read ISR
			std::uint8_t isr = inp(0x20);
			if (isr & 0x80 == 0) {
				return false; }}
		return true; }

	inline int GetISRNum() const {
		return isrNum_; }

	inline void Disconnect() const {
		outp(maskPort_, (inp(maskPort_)|stopMask_)); }

	inline void Connect() const {
		outp(maskPort_, (inp(maskPort_)&startMask_)); }

private:
	int controllerNum_;
	int rotatePort_;
	int maskPort_;
	int irqNum_;
	std::uint8_t isrNum_;
	std::uint8_t stopMask_;
	std::uint8_t startMask_; };


}  // namespace pic
}  // namespace rqdq
