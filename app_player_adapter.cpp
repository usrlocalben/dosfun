#include "app_player_adapter.hpp"

#include "kb_tinymod.hpp"
#ifdef SHOW_TIMING
#include "vga_reg.hpp"
#endif

#include <algorithm>
#include <cstdint>
#include <limits>

using std::int8_t;
using std::uint8_t;
using std::int16_t;
using std::uint16_t;

namespace rqdq {
namespace {

inline int16_t FP16toS16(int x) {
	return std::clamp(x>>1, -32768, 32767); }

inline uint8_t FP16toU8(int x) {
	return std::clamp((x>>9)+128, 0, 255); }


}
namespace app {

void PlayerAdapter::BlasterJmp(void* out, int fmt, int numChannels, int numSamples, void* self) {
	static_cast<PlayerAdapter*>(self)->BlasterProc(out, fmt, numChannels, numSamples); }


inline
void PlayerAdapter::BlasterProc(void* out_, int fmt, int numChannels, int numSamples) {
#ifdef SHOW_TIMING
//vga::Color(255, { 0x80, 0xfc, 0x40 });
#endif
	if (rw_.Size() >= numSamples) {
		if (fmt == 16) {
			// 16-bit signed PCM
			int16_t* out = static_cast<int16_t*>(out_);
			for (int i=0; i<numSamples; i++) {
				int l, r;
				PopFront(l, r);
				if (numChannels == 2) {
					out[i*2+0] = FP16toS16(l);
					out[i*2+1] = FP16toS16(r); }
				else {
					out[i] = FP16toS16((l+r)/2); }}}
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
					out[i] = FP16toU8((l+r)/2); }}}}
#ifdef SHOW_TIMING
//vga::Color(255, { 0, 0, 0 });
#endif
	}


void PlayerAdapter::Refill() {
#ifdef SHOW_TIMING
vga::Color(255, { 0xc0, 0x40, 0x20 });
#endif
	// int numSamples = std::min(rw_.Available(), 256U);
	int numSamples = rw_.Available();
	if (numSamples > 0) {
		player_.Render(pbuf_, pbuf_+capacity, numSamples);
		for (int i=0; i<numSamples; i++) {
			int l = pbuf_[i];
			int r = pbuf_[i+capacity];
			PushBack(l, r); }}
#ifdef SHOW_TIMING
vga::Color(255, { 0, 0, 0 });
#endif
	}


}  // namespace app
}  // namespace rqdq
