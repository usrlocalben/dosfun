#pragma once
#include <cstdint>

using std::uint8_t;
using std::uint16_t;

namespace rqdq {

struct keyinfo {
	int scanCode;
	int ascii; };


keyinfo WaitForKey();

void InstallKeyboard();
void UninstallKeyboard();
bool IsKeyboardDataAvailable();


class Keyboard {
public:
	Keyboard() {
		InstallKeyboard(); }
	~Keyboard() {
		UninstallKeyboard(); }
	bool IsDataAvailable() {
		return IsKeyboardDataAvailable(); }};


}  // namespace rqdq
