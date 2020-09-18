#pragma once
#include <cstdint>

namespace rqdq {
namespace pc {

constexpr int SC_ESC = 1;

struct KeyEvent {
	int code;
	bool down; };

void InstallKeyboard();
void UninstallKeyboard();
auto IsKeyboardDataAvailable() -> bool;
auto GetKeyboardMessage() -> KeyEvent;


class Keyboard {
public:
	Keyboard();
	~Keyboard();
	// not copyable
	auto operator=(const Keyboard&) -> Keyboard& = delete;
	Keyboard(const Keyboard&);

public:
	auto IsDataAvailable() -> bool {
		return IsKeyboardDataAvailable(); }
	auto GetMessage() -> KeyEvent {
		return GetKeyboardMessage(); }};


}  // namespace pc
}  // namespace rqdq
