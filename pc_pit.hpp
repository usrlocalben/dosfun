#pragma once
#include <cstdint>
#include <conio.h>  // outp, inp
#include <i86.h>  // _disable, _enable

#include "pc_pic.hpp"

using std::uint8_t;
using std::uint16_t;

namespace rqdq {
namespace pc {

const int PIT_MAX_PERIOD_IN_TICKS = 65536;

const uint8_t PIT_VALUE_MODE_16_BIT_BINARY = 0;

const uint8_t PIT_MODE0 = 0;
const uint8_t PIT_MODE3 = 0x06;

const uint8_t PIT_ACCESS_MODE_LOW_THEN_HIGH = 0x30;
const uint8_t PIT_ACCESS_MODE_LATCH_COUNT_VALUE_COMMAND = 0;

const uint8_t PIT_CHANNEL_SELECT_CH0 = 0;

extern IRQLineCT<0> pitIRQLine;

inline uint8_t lowbyte(uint16_t value) {
	return value & 0xff; }


inline uint8_t highbyte(uint16_t value) {
	return value >> 8; }


inline float ticksToHz(uint16_t value) {
	// PIT clock speed is (105/88)*1Mhz
	return 105000000/(88.0*value); }


inline float ticksToSeconds(int value) {
	return (88.0*value)/105000000; }


inline void StartCountdown(uint16_t period) {
	// start timer
	outp(0x43, PIT_CHANNEL_SELECT_CH0 | PIT_ACCESS_MODE_LOW_THEN_HIGH | PIT_MODE0 | PIT_VALUE_MODE_16_BIT_BINARY);
	outp(0x40, lowbyte(period));
	outp(0x40, highbyte(period)); }


inline void BeginMeasuring() {
	StartCountdown(0); }


inline void StartSquareWave(uint16_t period) {
	outp(0x43, PIT_CHANNEL_SELECT_CH0 | PIT_ACCESS_MODE_LOW_THEN_HIGH | PIT_MODE3 | PIT_VALUE_MODE_16_BIT_BINARY);
	outp(0x40, lowbyte(period));
	outp(0x40, highbyte(period)); }


inline uint16_t ReadCounter() {
	_disable();
	outp(0x43, 0);  // latch counter for channel 0
	uint8_t lo = inp(0x40);
	uint8_t hi = inp(0x40);
	_enable();
	return hi<<8|lo; }


inline uint16_t EndMeasuring() {
	return PIT_MAX_PERIOD_IN_TICKS - ReadCounter(); }


class Stopwatch {
public:
	Stopwatch() {
		start_ = ReadCounter(); }

	float GetElapsedTimeInSeconds() {
		int now = ReadCounter();
		int elapsedTimeInTicks = start_ - now;  // PIT counts down
		return ticksToSeconds(elapsedTimeInTicks); }
private:
	int start_; };


}  // namespace pc
}  // namespace rqdq
