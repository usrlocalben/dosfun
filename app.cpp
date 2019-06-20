#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>

#include "kbd.hpp"
#include "vga.hpp"
#include "snd.hpp"
#include "mod.hpp"
#include "ost.hpp"

using std::int8_t;
using std::uint8_t;
using std::uint16_t;
using std::int16_t;
using std::cout;
using std::hex;
using std::dec;
using namespace rqdq;

struct VRAMPage {
	uint8_t* addr;
	uint16_t vgaAddr; };

const VRAMPage pages[2] = {
	{ vga::VGAPTR, 0 },
	{ vga::VGAPTR + (320*240/4), 320*240/4 } };

volatile int timeInFrames = 0;
volatile int8_t backPage = 1;
volatile bool backLocked = true;

void vbi() {
	++timeInFrames;
	if (!backLocked) {
		vga::SetStartAddress(pages[backPage].vgaAddr);
		backPage ^= 1;
		backLocked = true; }}

class VRAMLock {
public:
	VRAMLock() {
		locked_ = backLocked; }
	~VRAMLock() {
		if (locked_) {
			backLocked = false; }}
	const VRAMPage& Page() {
		return pages[backPage]; }
	bool IsLocked() {
		return locked_; }
private:
	bool locked_; };


void DrawKefrensBars(const VRAMPage dst, float T, int patternNum, int rowNum);


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

	uint8_t colorpos = 40;
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
		// const int workFactor = 80;
		// for (int i=0; i<workFactor; i++)
		DrawKefrensBars(vramLock.Page(), T, patternNum, rowNum);
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


void DrawKefrensBars(const VRAMPage dst, float T, int patternNum, int rowNum) {
	int whole = T;
	float frac = T - whole;

	const int goodLookingColorMagic[] = {
		0, 1, 2, 3, 4,
		6, 15, 18, 19, 21,
		24, 30, 33, 34, 36,
		39, 49, 51, 52 };

	// int magicIdx = patternNum<<1 | (rowNum>>4&1);
	int magicIdx = patternNum;
	uint8_t colorpos = goodLookingColorMagic[magicIdx%19] * 17;

	bool first = true;
	uint8_t* rowPtr = dst.addr;
	uint8_t* prevPtr = dst.addr - 80;
	for (int yyy=0; yyy<240; yyy++) {
		if (first) {
			first = false;
			vga::SelectPlanes(0xf);
			for (int xxx=0; xxx<80; xxx++) {
				rowPtr[xxx] = 0; }}
		else {
			vga::SelectPlanes(0xf);
			vga::SetBitMask(0x00);  // latches will write
			for (int xxx=0; xxx<80; xxx++) {
				volatile char latchload = prevPtr[xxx];
				rowPtr[xxx] = 0; }
			vga::SetBitMask(0xff); }  // normal write

		int pos;
		switch (patternNum%4) {
		case 0:
			pos = std::sin((T*2.19343) + yyy*(std::sin(T/4.0f)*0.05) * 3.14159 * 2.0) * (std::sin(T*1.781234)*150) + 160;
			break;
		case 1:
			pos = std::sin((T) + yyy*0.005 * 3.14159 * 2.0) * (std::sin(T*3.781234)*150) + 160;
			break;
		case 2:
			pos = std::sin((T*5.666) + yyy*0.008 * 3.14159 * 2.0) * (std::sin(T*1.781234+(yyy*0.010))*50+100) + std::sin((yyy+(T*60))*0.00898)*100+160;
			break;
		case 3:
			pos = std::sin((T*2.45) + yyy*0.012 * 3.14159 * 2.0) * (std::sin(T*1.781234+(yyy*0.010))*66+33) + std::sin((yyy+(T*60))*0.01111)*100+50;
			break; }

		// if (yyy%2==0)
		for (int wx=-4; wx<=4; wx++) {
			vga::PutPixelSlow(pos+wx, yyy, colorpos+wx, dst.addr); }

		prevPtr = rowPtr;
		rowPtr += 80; }}
