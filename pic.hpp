#pragma once
#include <cstdint>
#include <conio.h>  // inp/outp

namespace rqdq {
namespace pic {

struct IRQLine {
	int controllerNum;
	int rotatePort;
	int maskPort;
	int irqNum;
	std::uint8_t isrNum;
	std::uint8_t stopMask;
	std::uint8_t startMask; };


IRQLine make_irqline(int irqNum);


inline void SignalEOI(const IRQLine pi) {
	if (pi.irqNum >= 8) {
		outp(0xa0, 0x20); }
	outp(0x20, 0x20); }


inline bool IsRealIRQ(const IRQLine pi) {
	if (pi.irqNum == 7) {
		outp(0x20, 0x0b);  // read ISR
		std::uint8_t isr = inp(0x20);
		if (isr & 0x80 == 0) {
			return false; }}
	return true; }


inline void Stop(const IRQLine& irqLine) {
	outp(irqLine.maskPort, (inp(irqLine.maskPort)|irqLine.stopMask)); }


inline void Start(const IRQLine& irqLine) {
	outp(irqLine.maskPort, (inp(irqLine.maskPort)&irqLine.startMask)); }


}  // namespace pic
}  // namespace rqdq
