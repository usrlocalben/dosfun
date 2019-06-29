#include "pc_kbd.hpp"

#include <cstdint>

#include "pc_cpu.hpp"
#include "pc_pic.hpp"

using std::uint8_t;

namespace rqdq {
namespace pc {

const int KBD_B_DATA = 0x60;
const int KBD_B_CONTROL = 0x61;
const int KBD_IRQ_RESET = 0x80;

uint8_t inputBuffer[256];
volatile uint8_t bufHead = 0;
volatile uint8_t bufTail = 0;

IRQLineCT<1> kbdIRQLine;


void __interrupt keyboard_isr() {
	uint8_t scanCode = inp(KBD_B_DATA);
	uint8_t status = inp(KBD_B_CONTROL);
	TXdb(KBD_B_CONTROL, status | KBD_IRQ_RESET);
	TXdb(KBD_B_CONTROL, status);

	inputBuffer[bufTail++] = scanCode;
	kbdIRQLine.SignalEOI(); }



void InstallKeyboard() {
	kbdIRQLine.SaveISR();
	kbdIRQLine.SetISR(keyboard_isr); }


void UninstallKeyboard() {
	kbdIRQLine.RestoreISR(); }


bool IsKeyboardDataAvailable() {
	return !(bufHead == bufTail); }


Event GetKeyboardMessage() {
	uint8_t sc = inputBuffer[bufHead++];
	Event ke;
	ke.down = (sc&0x80) == 0;
	ke.scanCode = sc&0x7f;
	return ke; }


}  // namespace pc
}  // namespace rqdq
