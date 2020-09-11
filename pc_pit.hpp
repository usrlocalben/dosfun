#pragma once
#include "pc_cpu.hpp"
#include "pc_pic.hpp"

#include <cstdint>

namespace rqdq {
namespace pc {

constexpr int PIT_MAX_PERIOD_IN_TICKS = 65536;

constexpr std::uint8_t PIT_VALUE_MODE_16_BIT_BINARY = 0;

constexpr std::uint8_t PIT_MODE0 = 0;
constexpr std::uint8_t PIT_MODE3 = 0x06;

constexpr std::uint8_t PIT_ACCESS_MODE_LOW_THEN_HIGH = 0x30;
constexpr std::uint8_t PIT_ACCESS_MODE_LATCH_COUNT_VALUE_COMMAND = 0;

constexpr std::uint8_t PIT_CHANNEL_SELECT_CH0 = 0;

extern IRQLineCT<0> pitIRQLine;

inline
auto lowbyte(std::uint16_t value) -> std::uint8_t {
	return value & 0xff; }


inline
auto highbyte(std::uint16_t value) -> std::uint8_t {
	return value >> 8; }


inline
auto ticksToHz(std::uint16_t value) -> float {
	// PIT clock speed is (105/88)*1Mhz
	return 105000000/(88.0*value); }


inline
auto ticksToSeconds(int value) -> float {
	return (88.0*value)/105000000; }


inline void StartCountdown(std::uint16_t period) {
	// start timer
	OutB(0x43, PIT_CHANNEL_SELECT_CH0 | PIT_ACCESS_MODE_LOW_THEN_HIGH | PIT_MODE0 | PIT_VALUE_MODE_16_BIT_BINARY);
	OutB(0x40, lowbyte(period));
	OutB(0x40, highbyte(period)); }


inline void BeginMeasuring() {
	StartCountdown(0); }


inline void StartSquareWave(std::uint16_t period) {
	OutB(0x43, PIT_CHANNEL_SELECT_CH0 | PIT_ACCESS_MODE_LOW_THEN_HIGH | PIT_MODE3 | PIT_VALUE_MODE_16_BIT_BINARY);
	OutB(0x40, lowbyte(period));
	OutB(0x40, highbyte(period)); }


inline
auto ReadCounter() -> std::uint16_t {
	CriticalSection cs;
	OutB(0x43, 0);  // latch counter for channel 0
	std::uint8_t lo = InB(0x40);
	std::uint8_t hi = InB(0x40);
	return hi<<8|lo; }


inline
auto EndMeasuring() -> std::uint16_t {
	return PIT_MAX_PERIOD_IN_TICKS - ReadCounter(); }


class Stopwatch {
	int start_;

public:
	Stopwatch() {
		start_ = ReadCounter(); }

	auto GetElapsedTimeInSeconds() -> float {
		int now = ReadCounter();
		int elapsedTimeInTicks = start_ - now;  // PIT counts down
		return ticksToSeconds(elapsedTimeInTicks); }};


}  // namespace pc
}  // namespace rqdq
