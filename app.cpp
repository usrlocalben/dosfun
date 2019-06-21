#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <limits>
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

const int kAudioBufferSizeInSamples = 128;
const int kAudioSampleRateInHz = 22050;
const int kAudioWidthInChannels = 2;
const int kSoundBlasterIOBaseAddr = 0x220;
const int kSoundBlasterIRQNum = 0x7;
const int kSoundBlasterDMAChannelNum = 0x05;


float measuredRefreshRateInHz;

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




float pbuf[4096*2];

mod::Player* thePlayer;

void audiostream(int16_t* out, int numChannels, int numSamples) {
#ifdef SHOW_TIMING
vga::SetRGB(0, 0x20, 0x3f, 0x10);
#endif
	thePlayer->Render(pbuf, pbuf+4096, numSamples);
#ifdef SHOW_TIMING
vga::SetRGB(0, 0, 0, 0);
#endif
	for (int i=0; i<numSamples; i++) {
		if (numChannels == 2) {
			out[i*2+0] = pbuf[i]      * std::numeric_limits<int16_t>::max();
			out[i*2+1] = pbuf[i+4096] * std::numeric_limits<int16_t>::max(); }
		else {
			out[i] = ((pbuf[i]+pbuf[i+4096])*0.5f) * std::numeric_limits<int16_t>::max(); }}}


void Demo() {
	kbd::Keyboard kbd;

	vga::ModeSetter modeSetter;
	modeSetter.Set(vga::VM_MODEX);
	vga::SoftVBI softVBI(&vbi);
	measuredRefreshRateInHz = softVBI.GetFrequency();

	mod::Paula paula;
	thePlayer = new mod::Player(&paula, (uint8_t*)ostData);
	snd::Blaster blaster(kSoundBlasterIOBaseAddr,
	                     kSoundBlasterIRQNum,
	                     kSoundBlasterDMAChannelNum,
	                     kAudioSampleRateInHz,
						 kAudioWidthInChannels,
	                     kAudioBufferSizeInSamples);
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

		float T = timeInFrames / measuredRefreshRateInHz;
		int patternNum = thePlayer->GetCurrentPos();
		int rowNum = thePlayer->GetCurrentRow();

#ifdef SHOW_TIMING
		vga::SetRGB(0, 0x30, 0x30, 0x30);
#endif
		for (int i=0; i<1; i++)
		efx::DrawKefrensBars(vramLock.Page(), T, patternNum, rowNum);
#ifdef SHOW_TIMING
		vga::SetRGB(0, 0,0,0);
#endif
		}}


int main(int argc, char *argv[]) {
	Demo();
	std::cout << "        elapsedTime: " << timeInFrames << " frames\n";
	std::cout << "measuredRefreshRate:   " << measuredRefreshRateInHz << " hz\n";
	return 0; }
