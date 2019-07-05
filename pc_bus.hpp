#pragma once
#include <cstdint>

#include <pc.h>  // inp/inpw/outp/outpw

namespace rqdq {
namespace pc {

inline void OutB(std::uint16_t port, std::uint8_t value) {
	outp(port, value); }


inline void OutW(std::uint16_t port, std::uint16_t value) {
	outpw(port, value); }


inline std::uint8_t InB(std::uint16_t port) {
	return inp(port); }


inline std::uint16_t InW(std::uint16_t port) {
	return inpw(port); }


}  // namespace pc
}  // namespace rqdq
