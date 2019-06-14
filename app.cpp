#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>

#include "kbd.hpp"
#include "vga.hpp"
#include "snd.hpp"

using std::uint8_t;
using std::uint16_t;
using std::int16_t;
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

volatile int st=0;

int16_t saw[50];

void audiostream(int16_t* buf, int len) {
	for (int i=0; i<len; i++) {
		st++;
		int16_t val = saw[st%50];
		buf[i] = val; }}

DemoStats Demo() {

	// generate a sawtooth test tone
	// 22050 / 440hz = ~50.11
	for (int i=0; i<50; i++) {
		saw[i] = (i/50.0) * 60000 - 32767; }

	DemoStats stats;
	Keyboard kbd;

	SetModeX();
	
	SoftVBI softVBI(&vbi);
	stats.measuredRefreshRateInHz = softVBI.GetFrequency();

	Blaster blaster(0x220, 7, 5, 22050);
	blaster.AttachProc(&audiostream);

	uint8_t act = 0;
	while (!kbd.IsDataAvailable()) {
		if (backbufferReady == false) {
			DrawBorder();

			float T = Tf * (1.0/stats.measuredRefreshRateInHz);
			T /= 2;
			int whole = T;
			float frac = T - whole;
			int pos = std::sin(frac*3.14159*2.0)*40 + 160;

			uint8_t color = whole;

			for (int by=120; by<=140; by++) {
				for (int bx=pos; bx<=pos+20; bx++) {
					PutPixelSlow(bx, by, color, vgaBack); }}
			// vgaBack[pos + (80*120)] = color;

			backbufferReady = true; }}

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
