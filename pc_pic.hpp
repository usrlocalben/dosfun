/*
 * "driver" for Intel 8259 PIC
 */
#pragma once
#include <cstdint>
#include <conio.h>  // inp/outp
#include <dos.h>  // _dos_setvect/getvect

namespace rqdq {
namespace pc {

typedef void (__interrupt * isrptr)();


class IRQLine {
public:
	IRQLine(int irqNum);

	inline void SignalEOI() const {
		// todo: optimizer won't eliminate this branch
		//       even when irqNum is const at compile-time
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

	void SetVect(isrptr func) const {
		_dos_setvect(isrNum_, func); }

	isrptr GetVect() const {
		return _dos_getvect(isrNum_); }

	void SaveVect() {
		savedISRPtr_ = GetVect(); }

	void RestoreVect() {
		SetVect(savedISRPtr_); }

private:
	const int irqNum_;
	int controllerNum_;
	int rotatePort_;
	int maskPort_;
	std::uint8_t isrNum_;
	std::uint8_t stopMask_;
	std::uint8_t startMask_;
	isrptr savedISRPtr_; };


}  // namespace pc
}  // namespace rqdq
