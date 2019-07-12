/*
 * "driver" for Intel 8259 PIC
 */
#pragma once
#include <cstdint>
#include <utility>
#include <go32.h>  // go32_my_cs

#include "pc_bus.hpp"
#include "pc_cpu.hpp"

namespace rqdq {
namespace pc {


template <int IRQNUM>
class IRQLineCT {
public:
	IRQLineCT()
		:irqNum_(IRQNUM),
		savedISRPtr_(),
		controllerNum_(IRQNUM < 8 ? 1 : 2),
		rotatePort_(IRQNUM < 8 ? 0x20 : 0xa0),
		maskPort_(IRQNUM < 8 ? 0x21 : 0xa1),
		isrNum_(IRQNUM < 8 ? 0x08+IRQNUM : 0x70 + IRQNUM - 8),
		stopMask_(1 << (IRQNUM % 8)),
		startMask_(~stopMask_) {}

	void SignalEOI() const {
		// todo: optimizer won't eliminate this branch
		//       even when irqNum is const at compile-time
		if (IRQNUM >= 8) {
			OutB(0xa0, 0x20); }
		OutB(0x20, 0x20); }

	bool IsReal() const {
		if (IRQNUM == 7) {
			OutB(0x20, 0x0b);  // read ISR
			std::uint8_t isr = InB(0x20);
			if (isr & 0x80 == 0) {
				return false; }}
		return true; }

	int GetISRNum() const {
		return isrNum_; }

	void Disconnect() const {
		OutB(maskPort_, (InB(maskPort_)|stopMask_)); }

	void Connect() const {
		OutB(maskPort_, (InB(maskPort_)&startMask_)); }

	void SetISR(ISRFunc func) {
		PreparedISR newISR(func);
		SetVect(isrNum_, newISR);
		customISR_ = std::move(newISR); }

	ISRPtr GetISR() const {
		return GetVect(isrNum_); }

	void SaveISR() {
		savedISRPtr_ = GetISR(); }

	void RestoreISR() {
		SetVect(isrNum_, savedISRPtr_); }

private:
	const int irqNum_;
	const int controllerNum_;
	const int rotatePort_;
	const int maskPort_;
	const std::uint8_t isrNum_;
	const std::uint8_t stopMask_;
	const std::uint8_t startMask_;
	ISRPtr savedISRPtr_;
	PreparedISR customISR_; };


class IRQLineRT {
public:
	IRQLineRT(int irqNum);

	void SignalEOI() const {
		// todo: optimizer won't eliminate this branch
		//       even when irqNum is const at compile-time
		if (irqNum_ >= 8) {
			OutB(0xa0, 0x20); }
		OutB(0x20, 0x20); }

	bool IsReal() const {
		if (irqNum_ == 7) {
			OutB(0x20, 0x0b);  // read ISR
			std::uint8_t isr = InB(0x20);
			if (isr & 0x80 == 0) {
				return false; }}
		return true; }

	int GetISRNum() const {
		return isrNum_; }

	void Disconnect() const {
		OutB(maskPort_, (InB(maskPort_)|stopMask_)); }

	void Connect() const {
		OutB(maskPort_, (InB(maskPort_)&startMask_)); }

	void SetISR(ISRFunc func) {
		PreparedISR newISR(func);
		SetVect(isrNum_, newISR);
		customISR_ = std::move(newISR); }

	ISRPtr GetISR() const {
		return GetVect(isrNum_); }

	void SaveISR() {
		savedISRPtr_ = GetISR(); }

	void RestoreISR() {
		SetVect(isrNum_, savedISRPtr_); }

private:
	const int irqNum_;
	const int controllerNum_;
	const int rotatePort_;
	const int maskPort_;
	const std::uint8_t isrNum_;
	const std::uint8_t stopMask_;
	const std::uint8_t startMask_;
	ISRPtr savedISRPtr_;
	PreparedISR customISR_; };

inline uint16_t GetPICMasks() {
	uint8_t lo = InB(0x21);
	uint8_t hi = InB(0xa1);
	return hi<<8|lo; }


}  // namespace pc
}  // namespace rqdq
