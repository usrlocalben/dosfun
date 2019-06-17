#pragma once
#include <cstdint>

namespace rqdq {

struct KeyEvent {
	int scanCode;
	bool down; };

// keyinfo WaitForKey();

const int SC_ESC = 1;

void InstallKeyboard();
void UninstallKeyboard();
bool IsKeyboardDataAvailable();
KeyEvent GetKeyboardMessage();


class Keyboard {
public:
	Keyboard() {
		InstallKeyboard(); }
	~Keyboard() {
		UninstallKeyboard(); }
	bool IsDataAvailable() {
		return IsKeyboardDataAvailable(); }
	KeyEvent GetMessage() {
		return GetKeyboardMessage(); }};


}  // namespace rqdq
