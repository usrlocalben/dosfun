#include "pc_dma.hpp"

#include <cstdint>
#include <iostream>
#include <conio.h>  // inp/outp

#include "os_realmem.hpp"

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

namespace rqdq {
namespace {

uint8_t lo(uint16_t value) { return value & 0x00ff; }
uint8_t hi(uint16_t value) { return value >> 8; }


}  // namespace

namespace pc {

DMABuffer::DMABuffer(std::uint16_t sizeInWords)
	:realMem_(sizeInWords*2*2),
	sizeInWords_(sizeInWords) {

	uint32_t phy = realMem_.segment_ * 16;
	uint32_t rel = phy % 65536;
	if ((rel + (sizeInWords*2)) > 65536) {
		// if the start addr would cross
		// a 64k page boundary, advance
		// to the start of the next page
		phy = (phy+65536) & 0xff0000; }

	addr_ = phy; }


Channel make_channel(int dmaChannelNum) {
	Channel out;
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


void Configure(const Channel ch, const DMABuffer& buf) {
	outp(ch.maskPort, ch.stopMask);
	outp(ch.clearPtrPort, 0x00);
	outp(ch.modePort, ch.mode);
	outp(ch.baseAddrPort, lo(buf.Offset16()));
	outp(ch.baseAddrPort, hi(buf.Offset16()));
	outp(ch.countPort, lo(buf.sizeInWords_ - 1));
	outp(ch.countPort, hi(buf.sizeInWords_ - 1));
	outp(ch.pagePort, buf.Page());
	outp(ch.maskPort, ch.startMask); }


void Stop(const Channel ch) {
	outp(ch.maskPort, ch.stopMask); }


}  // namespace pc
}  // namespace rqdq
