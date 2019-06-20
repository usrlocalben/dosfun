#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <i86.h>  // _disable/_enable

#include "kbd.hpp"
#include "vga.hpp"
#include "snd.hpp"
#include "mod.hpp"
#include "ost.hpp"
#include "efx.hpp"

using std::int8_t;
using std::uint8_t;
using std::uint16_t;
using std::int16_t;
using std::cout;
using std::hex;
using std::dec;
using namespace rqdq;

volatile int timeInFrames = 0;
volatile int8_t backPage = 1;
volatile bool backLocked = true;

void vbi() {
	++timeInFrames;
	if (!backLocked) {
		vga::SetStartAddress(vga::modeXPages[backPage].vgaAddr);
		backPage ^= 1;
		backLocked = true; }}

class VRAMLock {
public:
	VRAMLock() {
		locked_ = backLocked; }
	~VRAMLock() {
		if (locked_) {
			backLocked = false; }}
	const vga::VRAMPage& Page() {
		return vga::modeXPages[backPage]; }
	bool IsLocked() {
		return locked_; }
private:
	bool locked_; };


struct DemoStats {
	float measuredRefreshRateInHz; };


float audioBuf[4096];

mod::Player* thePlayer;

void audiostream(int16_t* buf, int len) {
#ifdef SHOW_TIMING
vga::SetRGB(0, 0x20, 0x3f, 0x10);
#endif
	thePlayer->Render(audioBuf, len);
#ifdef SHOW_TIMING
vga::SetRGB(0, 0,0,0);
#endif
	for (int i=0; i<len; i++) {
		int16_t val = audioBuf[i] * 32767.0;
		buf[i] = val; }}


DemoStats Demo() {
	DemoStats stats;
	kbd::Keyboard kbd;

	mod::Paula paula;
	thePlayer = new mod::Player(&paula, (uint8_t*)ostData);

	vga::SetModeX();
	
	vga::SoftVBI softVBI(&vbi);
	stats.measuredRefreshRateInHz = softVBI.GetFrequency();

	snd::Blaster blaster(0x220, 7, 5, 22050);
	blaster.AttachProc(&audiostream);

	int lastSongPos = -1;
	while (1) {
		if (kbd.IsDataAvailable()) {
			kbd::Event ke = kbd.GetMessage();
			if (ke.down && ke.scanCode == kbd::SC_ESC) {
				break; }}

		VRAMLock vramLock;
		if (!vramLock.IsLocked()) {
			continue; } // spin until back-buffer is locked

		float T = timeInFrames / stats.measuredRefreshRateInHz;
		int patternNum = thePlayer->GetCurrentPos();
		int rowNum = thePlayer->GetCurrentRow();

#ifdef SHOW_TIMING
		vga::SetRGB(0, 0x30, 0x30, 0x30);
#endif
		efx::DrawKefrensBars(vramLock.Page(), T, patternNum, rowNum);
#ifdef SHOW_TIMING
		vga::SetRGB(0, 0,0,0);
#endif
		}

	return stats; }


int main(int argc, char *argv[]) {
	DemoStats stats = Demo();
	vga::SetBIOSMode(0x3);
	std::cout << "        elapsedTime: " << timeInFrames << " frames\n";
	std::cout << "measuredRefreshRate:   " << stats.measuredRefreshRateInHz << " hz\n";
	return 0; }
