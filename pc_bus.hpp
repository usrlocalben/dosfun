#pragma once
#include <bitset>
#include <cstdint>

#include <inlines/pc.h>  // inp/inpw/outp/outpw

namespace rqdq {
namespace pc {

inline
void OutB(std::uint16_t port, std::uint8_t value) {
	outportb(port, value); }


inline
void OutB(std::uint16_t port, std::bitset<8> value) {
	outportb(port, value.to_ulong()); }


inline
void OutW(std::uint16_t port, std::uint16_t value) {
	outportw(port, value); }


inline
void OutW(std::uint16_t port, std::bitset<16> value) {
	outportw(port, value.to_ulong()); }


inline
auto InB(std::uint16_t port) -> std::uint8_t {
	return inportb(port); }


inline
auto InW(std::uint16_t port) -> std::uint16_t {
	return inportw(port); }


}  // namespace pc
}  // namespace rqdq
