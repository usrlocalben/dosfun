#pragma once
#include <cstdint>

#include <conio.h>  // inp/inpw/outp/outpw

namespace rqdq {
namespace pc {

inline void TXdb(std::uint16_t port, std::uint8_t value) {
	outp(port, value); }


inline void TXdw(std::uint16_t port, std::uint16_t value) {
	outpw(port, value); }


inline std::uint8_t RXdb(std::uint16_t port) {
	return inp(port); }


inline std::uint16_t RXdw(std::uint16_t port) {
	return inpw(port); }


}  // namespace pc
}  // namespace rqdq
