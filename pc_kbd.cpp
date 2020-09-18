#include "pc_kbd.hpp"

#include "log.hpp"
#include "pc_cpu.hpp"
#include "pc_pic.hpp"

#include <cstdint>
#include <iostream>

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


void keyboard_isr() {
	uint8_t code = InB(KBD_B_DATA);
	uint8_t status = InB(KBD_B_CONTROL);
	OutB(KBD_B_CONTROL, status | KBD_IRQ_RESET);
	OutB(KBD_B_CONTROL, status);

	inputBuffer[bufTail++] = code;
	kbdIRQLine.SignalEOI(); }



void InstallKeyboard() {
	kbdIRQLine.SaveISR();
	kbdIRQLine.SetISR(keyboard_isr);
	log::info("kbd installed"); }


void UninstallKeyboard() {
	kbdIRQLine.RestoreISR();
	log::info("kbd released"); }


bool IsKeyboardDataAvailable() {
	return !(bufHead == bufTail); }


auto GetKeyboardMessage() -> KeyEvent {
	uint8_t sc = inputBuffer[bufHead++];
	KeyEvent ke;
	ke.down = (sc&0x80) == 0;
	ke.code = sc&0x7f;
	return ke; }


Keyboard::Keyboard() {
	InstallKeyboard(); }


Keyboard::~Keyboard() {
	std::cerr << "Keyboard::dtor\n";
	UninstallKeyboard(); }

}  // namespace pc
}  // namespace rqdq
