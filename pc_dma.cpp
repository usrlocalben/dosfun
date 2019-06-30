#include "pc_dma.hpp"

#include <cstdint>
#include <iostream>

#include "os_realmem.hpp"
#include "pc_bus.hpp"

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

namespace rqdq {
namespace {

inline uint8_t lo(uint16_t value) { return value & 0x00ff; }
inline uint8_t hi(uint16_t value) { return value >> 8; }


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


const int pagePorts[8] = {
	0x87, 0x83, 0x81, 0x82,
	0x00, 0x8b, 0x89, 0x8a };


DMAChannel::DMAChannel(int dcn)
	:controllerNum_(dcn < 4 ? 0 : 1),
	channelNum_(dcn < 4 ? dcn : dcn - 4),
	ioBase_(controllerNum_ == 0 ? 0x00 : 0xc0),
	stride_(controllerNum_ == 0 ? 1 : 2),
	maskPort_(ioBase_ + 0x0a*stride_),
	modePort_(ioBase_ + 0x0b*stride_),
	clearPtrPort_(ioBase_ + 0x0c*stride_),
	baseAddrPort_(ioBase_+channelNum_*2*stride_),
	countPort_(ioBase_+channelNum_*2*stride_+stride_),
	pagePort_(pagePorts[dcn]),
	stopMask_(channelNum_ + 0x04),
	startMask_(channelNum_ + 0x00),
	mode_(channelNum_ + 0x58) {}  // 010110xx


DMAChannel::~DMAChannel() {
	Stop(); }


void DMAChannel::Setup(const DMABuffer& buf) const {
	Stop();
	ClearFlipFlop();
	SetMode();
	SetMemoryAddr(buf);
	SetMemorySize(buf);
	Start(); }


inline void DMAChannel::ClearFlipFlop() const {
	TXdb(clearPtrPort_, 0x00); }


inline void DMAChannel::SetMode() const {
	TXdb(modePort_, mode_); }


void DMAChannel::SetMemoryAddr(const DMABuffer& buf) const {
	if (controllerNum_ == 0) {
		TXdb(baseAddrPort_, lo(buf.addr_%65536));
		TXdb(baseAddrPort_, hi(buf.addr_%65536)); }
	else {
		TXdb(baseAddrPort_, lo(buf.Offset16()));
		TXdb(baseAddrPort_, hi(buf.Offset16())); }
	TXdb(pagePort_, buf.Page()); }


void DMAChannel::SetMemorySize(const DMABuffer& buf) const {
	if (controllerNum_ == 0) {
		TXdb(countPort_, lo(buf.sizeInWords_*2 - 1));
		TXdb(countPort_, hi(buf.sizeInWords_*2 - 1)); }
	else {
		TXdb(countPort_, lo(buf.sizeInWords_ - 1));
		TXdb(countPort_, hi(buf.sizeInWords_ - 1)); }}


void DMAChannel::Stop() const {
	TXdb(maskPort_, stopMask_); }


void DMAChannel::Start() const {
	TXdb(maskPort_, startMask_); }


}  // namespace pc
}  // namespace rqdq
