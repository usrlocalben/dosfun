#include "app_player_adapter.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>

#include "kb_tinymod.hpp"
#ifdef SHOW_TIMING
#include "vga_reg.hpp"
#endif

using std::int8_t;
using std::uint8_t;
using std::int16_t;
using std::uint16_t;

namespace rqdq {
namespace {

inline std::int16_t FP16toS16(int x) {
	x >>= 1;
	x = std::min(std::max(x, -32768), 32767);
	return x; }


inline std::uint8_t FP16toU8(int x) {
	x >>= 9;
	x += 128;
	x = std::min(std::max(x, 0), 255);
	return x; }


}
namespace app {

void PlayerAdapter::BlasterJmp(void* out, int fmt, int numChannels, int numSamples, void* self) {
	static_cast<PlayerAdapter*>(self)->BlasterProc(out, fmt, numChannels, numSamples); }


inline void PlayerAdapter::BlasterProc(void* out_, int fmt, int numChannels, int numSamples) {
#ifdef SHOW_TIMING
//vga::SetRGB(0, 0x20, 0x3f, 0x10);
#endif
	if (rw_.Size() >= numSamples) {
		if (fmt == 2) {
			// 16-bit signed PCM
			int16_t* out = static_cast<int16_t*>(out_);
			for (int i=0; i<numSamples; i++) {
				int l, r;
				PopFront(l, r);
				if (numChannels == 2) {
					out[i*2+0] = FP16toS16(l);
					out[i*2+1] = FP16toS16(r); }
				else {
					out[i] = FP16toS16((l+r)>>1); }}}
		else {
			// 8-bit unsigned PCM
			uint8_t* out = static_cast<uint8_t*>(out_);
			for (int i=0; i<numSamples; i++) {
				int l, r;
				PopFront(l, r);
				if (numChannels == 2) {
					out[i*2+0] = FP16toU8(l);
					out[i*2+1] = FP16toU8(r); }
				else {
					out[i] = FP16toU8((l+r)>>1); }}}}
#ifdef SHOW_TIMING
//vga::SetRGB(0, 0, 0, 0);
#endif
	}


void PlayerAdapter::Refill() {
#ifdef SHOW_TIMING
vga::SetRGB(0, 0x30, 0x20, 0x10);
#endif
	// int numSamples = std::min(rw_.Available(), 128U);
	int numSamples = rw_.Available();
	if (numSamples > 0) {
		player_.Render(pbuf_, pbuf_+4096, numSamples);
		for (int i=0; i<numSamples; i++) {
			int l = pbuf_[i];
			int r = pbuf_[i+4096];
			PushBack(l, r); }}
#ifdef SHOW_TIMING
vga::SetRGB(0, 0, 0, 0);
#endif
	}


}  // namespace app
}  // namespace rqdq
