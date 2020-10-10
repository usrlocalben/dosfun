/*
 * "driver" for Intel 8259 PIC
 */
#pragma once
#include "pc_bus.hpp"
#include "pc_cpu.hpp"

#include <cstdint>
#include <utility>

#include <go32.h>  // go32_my_cs

namespace rqdq {
namespace pc {

template <int IRQNUM>
class IRQLineCT {

	constexpr static int controllerNum = IRQNUM < 8 ? 1 : 2;
	constexpr static int rotatePort = IRQNUM < 8 ? 0x20 : 0xa0;
	constexpr static int maskPort = IRQNUM < 8 ? 0x21 : 0xa1;
	constexpr static std::uint8_t isrNum = IRQNUM < 8 ? 0x08+IRQNUM : 0x70 + IRQNUM - 8;
	constexpr static int irqNum = IRQNUM;
	constexpr static std::uint8_t stopMask = 1 << (IRQNUM % 8);
	constexpr static std::uint8_t startMask = ~stopMask;

	ISRPtr savedISRPtr_{};
	PreparedISR customISR_;

public:
	constexpr IRQLineCT() = default;

	static
	void SignalEOI() {
		if (IRQNUM >= 8) {
			OutB(0xa0, 0x20); }
		OutB(0x20, 0x20); }

	constexpr static
	auto IsReal() -> bool {
		if (IRQNUM == 7) {
			OutB(0x20, 0x0b);  // read ISR
			std::uint8_t isr = InB(0x20);
			if (isr & 0x80 == 0) {
				return false; }}
		return true; }

	constexpr static
	auto ISRNum() -> int {
		return isrNum; }

	static
	void Disconnect() {
		OutB(maskPort, (InB(maskPort)|stopMask)); }

	static
	void Connect() {
		OutB(maskPort, (InB(maskPort)&startMask)); }

	void SetISR(ISRFunc func) {
		PreparedISR newISR(func);
		SetVect(isrNum, newISR);
		customISR_ = std::move(newISR); }

	auto GetISR() const -> ISRPtr {
		return GetVect(isrNum); }

	void SaveISR() {
		savedISRPtr_ = GetISR(); }

	void RestoreISR() {
		SetVect(isrNum, savedISRPtr_); }};


class IRQLineRT {

	const int irqNum_;
	const int controllerNum_;
	const int rotatePort_;
	const int maskPort_;
	const std::uint8_t isrNum_;
	const std::uint8_t stopMask_;
	const std::uint8_t startMask_;
	ISRPtr savedISRPtr_;
	PreparedISR customISR_;

public:
	IRQLineRT(int irqNum);

	void SignalEOI() const {
		// todo: optimizer won't eliminate this branch
		//       even when irqNum is const at compile-time
		if (irqNum_ >= 8) {
			OutB(0xa0, 0x20); }
		OutB(0x20, 0x20); }

	auto IsReal() const -> bool {
		if (irqNum_ == 7) {
			OutB(0x20, 0x0b);  // read ISR
			std::uint8_t isr = InB(0x20);
			if (isr & 0x80 == 0) {
				return false; }}
		return true; }

	auto GetISRNum() const -> int {
		return isrNum_; }

	void Disconnect() const {
		OutB(maskPort_, (InB(maskPort_)|stopMask_)); }

	void Connect() const {
		OutB(maskPort_, (InB(maskPort_)&startMask_)); }

	void SetISR(ISRFunc func) {
		PreparedISR newISR(func);
		SetVect(isrNum_, newISR);
		customISR_ = std::move(newISR); }

	auto GetISR() const -> ISRPtr {
		return GetVect(isrNum_); }

	void SaveISR() {
		savedISRPtr_ = GetISR(); }

	void RestoreISR() {
		SetVect(isrNum_, savedISRPtr_); }};


inline
auto GetPICMasks() -> std::uint16_t {
	std::uint8_t lo = InB(0x21);
	std::uint8_t hi = InB(0xa1);
	return hi<<8|lo; }


}  // namespace pc
}  // namespace rqdq
