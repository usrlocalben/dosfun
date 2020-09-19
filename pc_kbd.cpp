#include "pc_kbd.hpp"

#include "alg_ringindex.hpp"
#include "log.hpp"
#include "pc_cpu.hpp"
#include "pc_pic.hpp"

#include <array>
#include <cstdint>

using std::uint8_t;

namespace rqdq {
namespace {

using namespace rqdq::pc;

constexpr int KBD_B_DATA = 0x60;
constexpr int KBD_B_CONTROL = 0x61;
constexpr int KBD_IRQ_RESET = 0x80;

std::array<uint8_t, 256> inputBuffer;
alg::RingIndex<256> inputIndex;

IRQLineCT<1> kbdIRQLine;

struct KeyboardUtil {

	static
	void isr() {
		uint8_t scanCode = InB(KBD_B_DATA);
		uint8_t status = InB(KBD_B_CONTROL);
		OutB(KBD_B_CONTROL, status | KBD_IRQ_RESET);
		OutB(KBD_B_CONTROL, status);

		inputBuffer[inputIndex.PushBack()] = scanCode;
		kbdIRQLine.SignalEOI(); }

	static
	void Install() {
		kbdIRQLine.SaveISR();
		kbdIRQLine.SetISR(KeyboardUtil::isr);
		log::info("kbd installed"); }

	static
	void Uninstall() {
		kbdIRQLine.RestoreISR();
		log::info("kbd released"); }

	static
	auto Loaded() -> bool {
		return inputIndex.Loaded(); }

	static
	auto Pop() -> KeyEvent {
		uint8_t sc = inputBuffer[inputIndex.PopFront()];
		KeyEvent ke;
		ke.down = (sc&0x80) == 0;
		ke.code = sc&0x7f;
		return ke; }};

}  // close unnamed namespace
namespace pc {

Keyboard::Keyboard() {
	KeyboardUtil::Install(); }


Keyboard::~Keyboard() {
	KeyboardUtil::Uninstall(); }


auto Keyboard::Loaded() -> bool {
	return KeyboardUtil::Loaded(); }


auto Keyboard::Pop() -> KeyEvent {
	return KeyboardUtil::Pop(); }


}  // namespace pc
}  // namespace rqdq
