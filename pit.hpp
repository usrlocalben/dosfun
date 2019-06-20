#pragma once
#include <cstdint>
#include <conio.h>  // outp, inp
#include <i86.h>  // _disable, _enable

using std::uint8_t;
using std::uint16_t;

namespace rqdq {
namespace pit {

const uint16_t PIT_MAX_PERIOD_IN_TICKS = 0xffff;

const uint8_t PIT_VALUE_MODE_16_BIT_BINARY = 0;

const uint8_t PIT_MODE0 = 0;
const uint8_t PIT_MODE3 = 0x06;

const uint8_t PIT_ACCESS_MODE_LOW_THEN_HIGH = 0x30;
const uint8_t PIT_ACCESS_MODE_LATCH_COUNT_VALUE_COMMAND = 0;

const uint8_t PIT_CHANNEL_SELECT_CH0 = 0;


inline uint8_t lowbyte(uint16_t value) {
	return value & 0xff; }


inline uint8_t highbyte(uint16_t value) {
	return value >> 8; }


inline float ticksToHz(uint16_t value) {
	return 105.0/88*1000000/value; }


inline void StartCountdown(uint16_t period) {
	// start timer
	outp(0x43, PIT_CHANNEL_SELECT_CH0 | PIT_ACCESS_MODE_LOW_THEN_HIGH | PIT_MODE0 | PIT_VALUE_MODE_16_BIT_BINARY);
	outp(0x40, lowbyte(period));
	outp(0x40, highbyte(period)); }


inline void BeginMeasuring() {
	StartCountdown(PIT_MAX_PERIOD_IN_TICKS); }


inline void StartSquareWave(uint16_t period) {
	outp(0x43, PIT_CHANNEL_SELECT_CH0 | PIT_ACCESS_MODE_LOW_THEN_HIGH | PIT_MODE3 | PIT_VALUE_MODE_16_BIT_BINARY);
	outp(0x40, lowbyte(period));
	outp(0x40, highbyte(period)); }


inline uint16_t EndMeasuring() {
	_disable();
	outp(0x43, 0);  // latch counter for channel 0
	uint8_t lo = inp(0x40);
	uint8_t hi = inp(0x40);
	_enable();
	return PIT_MAX_PERIOD_IN_TICKS - (hi<<8|lo); }


}  // namespace pit
}  // namespace rqdq
