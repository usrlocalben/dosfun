#pragma once
#include <cstdint>

namespace rqdq {
namespace pc {

const int SC_ESC = 1;

struct Event {
	int scanCode;
	bool down; };


void InstallKeyboard();
void UninstallKeyboard();
bool IsKeyboardDataAvailable();
Event GetKeyboardMessage();


class Keyboard {
public:
	Keyboard() { InstallKeyboard(); }
	~Keyboard() { UninstallKeyboard(); }
private:
	Keyboard& operator=(const Keyboard&);  // not copyable
	Keyboard(const Keyboard&);             // not copyable

public:
	bool IsDataAvailable() {
		return IsKeyboardDataAvailable(); }
	Event GetMessage() {
		return GetKeyboardMessage(); }};


}  // namespace pc
}  // namespace rqdq
