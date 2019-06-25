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


DMAChannel::DMAChannel(int dmaChannelNum) {
	maskPort = 0xd4;
	clearPtrPort = 0xd8;
	modePort = 0xd6;
	baseAddrPort = 0xc0 + 4*(dmaChannelNum-4);
	countPort = 0xc2 + 4*(dmaChannelNum-4);
	switch (dmaChannelNum) {
	case 5: pagePort = 0x8b; break;
	case 6: pagePort = 0x89; break;
	case 7: pagePort = 0x8a; break; }
	
	stopMask = dmaChannelNum-4 + 0x04;
	startMask = dmaChannelNum-4 + 0x00;
	mode = dmaChannelNum-4 + 0x58; }  // 010110xx


void DMAChannel::Setup(const DMABuffer& buf) const {
	outp(maskPort, stopMask);
	outp(clearPtrPort, 0x00);
	outp(modePort, mode);
	outp(baseAddrPort, lo(buf.Offset16()));
	outp(baseAddrPort, hi(buf.Offset16()));
	outp(countPort, lo(buf.sizeInWords_ - 1));
	outp(countPort, hi(buf.sizeInWords_ - 1));
	outp(pagePort, buf.Page());
	outp(maskPort, startMask); }


void DMAChannel::Stop() const {
	outp(maskPort, stopMask); }


}  // namespace pc
}  // namespace rqdq
