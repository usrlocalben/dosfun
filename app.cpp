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

using std::uint8_t;
using std::uint16_t;
using std::int16_t;
using std::cout;
using std::hex;
using std::dec;
using namespace rqdq;

volatile int Tf = 0;

void DrawBorder();
void DrawKefrensBars(float T, int patternNum, int rowNum);

uint8_t* const vgaA = vga::VGAPTR;
uint8_t* const vgaB = vga::VGAPTR + (320*240/4);
uint8_t* vgaBack = vgaB;
uint8_t* vgaFront = vgaA;

volatile bool backbufferReady = false;


void vbi() {
	Tf++;
	if (backbufferReady) {
		if (vgaBack == vgaB) {
			vga::SetStartAddress(320*240/4); }
		else {
			vga::SetStartAddress(0); }

		std::swap(vgaBack, vgaFront);
		backbufferReady = false; }}


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

// uint8_t modBits[4*1024*1024];

DemoStats Demo() {
	DemoStats stats;
	kbd::Keyboard kbd;

	/*
	bool done = false;
	while (!done) {
		while (!kbd.IsDataAvailable()) {}
		kbd::Event ke = kbd.GetMessage();
		cout << "key: " << (ke.down?"DOWN":" UP ") << " " << hex << int(ke.scanCode) << dec << "\n";  }
	*/

	/*
	std::ifstream fd("urea.mod", std::ios::in|std::ios::binary);
	fd.seekg(0, std::ios::end);
	std::streampos len(fd.tellg());
	fd.seekg(0, std::ios::beg);
	fd.read(modBits, len);
	thePlayer = new mod::Player(&paula, modBits);
	*/

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
		if (backbufferReady == true) {
			continue; } // spin until buffer flip

		float T = Tf * (1.0/stats.measuredRefreshRateInHz);
		int patternNum = thePlayer->GetCurrentPos();
		int rowNum = thePlayer->GetCurrentRow();

#ifdef SHOW_TIMING
		vga::SetRGB(0, 0x30, 0x30, 0x30);
#endif
		DrawKefrensBars(T, patternNum, rowNum);
#ifdef SHOW_TIMING
		vga::SetRGB(0, 0,0,0);
#endif

		backbufferReady = true;

		if (kbd.IsDataAvailable()) {
			kbd::Event ke = kbd.GetMessage();
			if (ke.down && ke.scanCode == kbd::SC_ESC) {
				break; }}}

	return stats; }


int main(int argc, char *argv[]) {
	DemoStats stats = Demo();
	vga::SetBIOSMode(0x3);
	std::cout << "        elapsedTime: " << Tf << " frames\n";
	std::cout << "measuredRefreshRate:   " << stats.measuredRefreshRateInHz << " hz\n";
	return 0; }


void DrawBorder() {
	uint8_t* dst = vgaBack;
	vga::SelectAllPlanes();
	for (int y=0; y<240; y++) {
		for (int x=0; x<80; x++) {
			if (y==0 || y==239 || x==0 || x==79) {
				*dst = 2; }
			else {
				*dst = 0; }
			dst++; }}}


void DrawKefrensBars(float T, int patternNum, int rowNum) {
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
	uint8_t* rowPtr = vgaBack;
	uint8_t* prevPtr = vgaBack-80;
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
			pos = std::sin((T*5.666) + yyy*0.008 * 3.14159 * 2.0) * (std::sin(T*1.781234+(yyy*0.010))*50+100) + std::sin((yyy+Tf)*0.00898)*100+160;
			break;
		case 3:
			pos = std::sin((T*2.45) + yyy*0.012 * 3.14159 * 2.0) * (std::sin(T*1.781234+(yyy*0.010))*66+33) + std::sin((yyy+Tf)*0.01111)*100+50;
			break; }

		// if (yyy%2==0)
		for (int wx=-4; wx<=4; wx++) {
			vga::PutPixelSlow(pos+wx, yyy, colorpos+wx, vgaBack); }

		prevPtr = rowPtr;
		rowPtr += 80; }}
