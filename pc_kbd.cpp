#include "pc_kbd.hpp"

#include <cstdint>
#include <i86.h>  // int386
#include <dos.h>  // _dos_getvect, _dos_setvect
#include <conio.h>  // inp/outp

#include "pc_pic.hpp"

using std::uint8_t;

namespace rqdq {
namespace pc {

const int KBD_B_DATA = 0x60;
const int KBD_B_CONTROL = 0x61;
const int KBD_IRQ_RESET = 0x80;

const int KBD_IRQ_NUM = 1;

uint8_t inputBuffer[256];
volatile uint8_t bufHead = 0;
volatile uint8_t bufTail = 0;

IRQLine irqLine(KBD_IRQ_NUM);


void __interrupt keyboard_isr() {
	uint8_t scanCode = inp(KBD_B_DATA);
	uint8_t status = inp(KBD_B_CONTROL);
	outp(KBD_B_CONTROL, status | KBD_IRQ_RESET);
	outp(KBD_B_CONTROL, status);

	inputBuffer[bufTail++] = scanCode;
	irqLine.SignalEOI(); }



void InstallKeyboard() {
	irqLine.SaveVect();
	irqLine.SetVect(keyboard_isr); }


void UninstallKeyboard() {
	irqLine.RestoreVect(); }


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
