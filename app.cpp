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

uint8_t* const vgaA = VGAPTR;
uint8_t* const vgaB = VGAPTR + (320*240/4);
uint8_t* vgaBack = vgaB;
uint8_t* vgaFront = vgaA;

volatile bool backbufferReady = false;


void vbi() {
	Tf++;
	if (backbufferReady) {
		if (vgaBack == vgaB) {
			SetStartAddress(320*240/4); }
		else {
			SetStartAddress(0); }

		std::swap(vgaBack, vgaFront);
		backbufferReady = false; }}


struct DemoStats {
	float measuredRefreshRateInHz; };

float audioBuf[4096];

ModPlayer* thePlayer;

void audiostream(int16_t* buf, int len) {
	thePlayer->Render(audioBuf, len);
	for (int i=0; i<len; i++) {
		int16_t val = audioBuf[i] * 32767.0;
		buf[i] = val; }}

// uint8_t modBits[4*1024*1024];

DemoStats Demo() {
	DemoStats stats;
	Keyboard kbd;

	/*
	bool done = false;
	while (!done) {
		while (!kbd.IsDataAvailable()) {}
		KeyEvent ke = kbd.GetMessage();
		cout << "key: " << (ke.down?"DOWN":" UP ") << " " << hex << int(ke.scanCode) << dec << "\n";  }
	*/

	/*
	std::ifstream fd("urea.mod", std::ios::in|std::ios::binary);
	fd.seekg(0, std::ios::end);
	std::streampos len(fd.tellg());
	fd.seekg(0, std::ios::beg);
	fd.read(modBits, len);
	thePlayer = new ModPlayer(&paula, modBits);
	*/

	Paula paula;
	thePlayer = new ModPlayer(&paula, (uint8_t*)ostData);


	SetModeX();
	
	SoftVBI softVBI(&vbi);
	stats.measuredRefreshRateInHz = softVBI.GetFrequency();


	Blaster blaster(0x220, 7, 5, 22050);
	blaster.AttachProc(&audiostream);

	uint8_t act = 0;
	while (1) {
		if (backbufferReady == true) {
			continue; } // spin until buffer flip

		// DrawBorder();

		float T = Tf * (1.0/stats.measuredRefreshRateInHz);
		//T /= 2;
		int whole = T;
		float frac = T - whole;

		bool first = true;
		uint8_t* rowPtr = vgaBack;
		uint8_t* prevPtr = vgaBack-80;
		for (int yyy=0; yyy<240; yyy++) {
			if (first) {
				first = false;
				SelectPlanes(0xf);
				for (int xxx=0; xxx<80; xxx++) {
					rowPtr[xxx] = 0; }}
			else {
				SelectPlanes(0xf);
				SetBitMask(0x00);  // latches will write
				for (int xxx=0; xxx<80; xxx++) {
					volatile char latchload = prevPtr[xxx];
					rowPtr[xxx] = 0; }
				SetBitMask(0xff); }  // normal write

			int pos = std::sin((T*2.19343) + yyy*(std::sin(T/4.0f)*0.05) * 3.14159 * 2.0) * (std::sin(T*1.781234)*150) + 160;
			for (int wx=-3; wx<=3; wx++) {
				PutPixelSlow(pos+wx, yyy, 40+wx, vgaBack); }

			prevPtr = rowPtr;
			rowPtr += 80; }

		backbufferReady = true;

		if (kbd.IsDataAvailable()) {
			KeyEvent ke = kbd.GetMessage();
			if (ke.down && ke.scanCode == SC_ESC) {
				break; }}}

	return stats; }


int main(int argc, char *argv[]) {
	DemoStats stats = Demo();
	SetBIOSMode(0x3);
	std::cout << "        elapsedTime: " << Tf << " frames\n";
	std::cout << "measuredRefreshRate:   " << stats.measuredRefreshRateInHz << " hz\n";
	return 0; }


void DrawBorder() {
	uint8_t* dst = vgaBack;
	SelectAllPlanes();
	for (int y=0; y<240; y++) {
		for (int x=0; x<80; x++) {
			if (y==0 || y==239 || x==0 || x==79) {
				*dst = 2; }
			else {
				*dst = 0; }
			dst++; }}}
