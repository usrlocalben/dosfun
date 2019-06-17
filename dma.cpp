#include "dma.hpp"

#include <cstdint>
#include <iostream>
#include <conio.h>  // inp/outp

#include "mem.hpp"

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

namespace {

uint8_t lo(uint16_t value) { return value & 0x00ff; }
uint8_t hi(uint16_t value) { return value >> 8; }


}  // namespace

namespace rqdq {

DMAPtr AllocDMABuffer(std::uint16_t sizeInWords) {
	DMAPtr out;
	out.sizeInWords = sizeInWords;
	out.realPtr = AllocReal(sizeInWords * 2 * 2);

	uint32_t phy = out.realPtr.segment * 16;
	uint32_t rel = phy % 65536;
	if ((rel + (sizeInWords*2)) > 65536) {
		// if the start addr would cross
		// a 64k page boundary, advance
		// to the start of the next page
		phy = (phy+65536) & 0xff0000; }

	out.addr = phy;
	return out; }


void FreeDMABuffer(DMAPtr ptr) {
	FreeReal(ptr.realPtr); }


DMAInfo make_dmainfo(int dmaChannelNum) {
	DMAInfo out;
	out.maskPort = 0xd4;
	out.clearPtrPort = 0xd8;
	out.modePort = 0xd6;
	out.baseAddrPort = 0xc0 + 4*(dmaChannelNum-4);
	out.countPort = 0xc2 + 4*(dmaChannelNum-4);
	switch (dmaChannelNum) {
	case 5: out.pagePort = 0x8b; break;
	case 6: out.pagePort = 0x89; break;
	case 7: out.pagePort = 0x8a; break; }
	
	out.stopMask = dmaChannelNum-4 + 0x04;
	out.startMask = dmaChannelNum-4 + 0x00;
	out.mode = dmaChannelNum-4 + 0x58;  // 010110xx
	return out; }


void ConfigureTransfer(DMAInfo di, DMAPtr mem) {
	outp(di.maskPort, di.stopMask);
	outp(di.clearPtrPort, 0x00);
	outp(di.modePort, di.mode);
	outp(di.baseAddrPort, lo(mem.Offset16()));
	outp(di.baseAddrPort, hi(mem.Offset16()));
	outp(di.countPort, lo(mem.sizeInWords - 1));
	outp(di.countPort, hi(mem.sizeInWords - 1));
	outp(di.pagePort, mem.Page());
	outp(di.maskPort, di.startMask); }


void StopDMA(DMAInfo di) {
	outp(di.maskPort, di.stopMask); }


}  // namespace rqdq
