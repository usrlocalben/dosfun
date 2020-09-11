#pragma once
#include <cstdint>

namespace rqdq {
namespace pc {

constexpr int SC_ESC = 1;

struct Event {
	int scanCode;
	bool down; };

void InstallKeyboard();
void UninstallKeyboard();
auto IsKeyboardDataAvailable() -> bool;
auto GetKeyboardMessage() -> Event;


class Keyboard {
public:
	Keyboard() { InstallKeyboard(); }
	~Keyboard() { UninstallKeyboard(); }
private:
	Keyboard& operator=(const Keyboard&);  // not copyable
	Keyboard(const Keyboard&);             // not copyable

public:
	auto IsDataAvailable() -> bool {
		return IsKeyboardDataAvailable(); }
	auto GetMessage() -> Event {
		return GetKeyboardMessage(); }};


}  // namespace pc
}  // namespace rqdq
