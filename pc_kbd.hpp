#pragma once

namespace rqdq {
namespace pc {

constexpr int SC_ESC  = 0x01;
constexpr int SC_UP   = 0x48;
constexpr int SC_DOWN = 0x50;

struct KeyEvent {
	int code;
	bool down; };


class Keyboard {
public:
	Keyboard();
	~Keyboard();
	// not copyable
	auto operator=(const Keyboard&) -> Keyboard& = delete;
	Keyboard(const Keyboard&) = delete;
	// movable
	auto operator=(Keyboard&&) -> Keyboard& = default;
	Keyboard(Keyboard&&) = default;

	auto Loaded() -> bool;
	auto Pop() -> KeyEvent; };


}  // namespace pc
}  // namespace rqdq
