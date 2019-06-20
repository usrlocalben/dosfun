#include "kbd.hpp"
#include <cstdint>
#include <i86.h>  // int386
#include <dos.h>  // _dos_getvect, _dos_setvect
#include <conio.h>  // inp/outp

using std::uint8_t;

namespace rqdq {
namespace kbd {

/*
keyinfo WaitForKey() {
	union REGS r;
	r.x.eax = 0;
	int386(0x16, &r, &r);
	keyinfo ki;
	ki.scanCode = r.h.ah;
	ki.ascii = r.h.al;
	return ki; }
*/

const int KBD_B_DATA = 0x60;
const int KBD_B_CONTROL = 0x61;
const int KBD_IRQ_RESET = 0x80;

const int KEYBOARD_ISR_NUM = 0x09;

uint8_t inputBuffer[256];
volatile uint8_t bufHead = 0;
volatile uint8_t bufTail = 0;

void (__interrupt * oldKeyboardISRPtr)();

void __interrupt keyboard_isr() {
	uint8_t scanCode = inp(KBD_B_DATA);

	uint8_t status = inp(KBD_B_CONTROL);
	outp(KBD_B_CONTROL, status | KBD_IRQ_RESET);
	outp(KBD_B_CONTROL, status);

	outp(0x20, 0x20);  // reset PIC

	inputBuffer[bufTail++] = scanCode; }


void InstallKeyboard() {
	oldKeyboardISRPtr = _dos_getvect(KEYBOARD_ISR_NUM);
	_dos_setvect(KEYBOARD_ISR_NUM, &keyboard_isr); }


void UninstallKeyboard() {
	_dos_setvect(KEYBOARD_ISR_NUM, oldKeyboardISRPtr); }


bool IsKeyboardDataAvailable() {
	return !(bufHead == bufTail); }


Event GetKeyboardMessage() {
	uint8_t sc = inputBuffer[bufHead++];
	Event ke;
	ke.down = (sc&0x80) == 0;
	ke.scanCode = sc&0x7f;
	return ke; }


}  // namespace kbd
}  // namespace rqdq
