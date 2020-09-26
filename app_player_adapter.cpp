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
		if (fmt == 2) {
			// 16-bit signed PCM
			int16_t* out = static_cast<int16_t*>(out_);
			if (numChannels == 1) {
				// mono
				for (int i=0; i<numSamples; ++i) {
					int16_t l, r;
					PopFront(l, r); // :14
					int m = (int32_t(l) + r); // :15
					m <<= 1;
					out[i] = m; }}
			else {
				// stereo
				for (int i=0; i<numSamples; ++i) {
					int16_t l, r;
					PopFront(l, r);
					l <<= 1;
					r <<= 1; // :15
					out[i*2+0] = l;
					out[i*2+1] = r; }}}
		else {
			// 8-bit unsigned PCM
			uint8_t* out = static_cast<uint8_t*>(out_);
			if (numChannels == 1) {
				// mono
				for (int i=0; i<numSamples; ++i) {
					int16_t l, r;
					PopFront(l, r); // :14
					int m = (int32_t(l) + r); // :15
					m >>= 8;
					m += 128;
					out[i] = m; }}
			else {
				// stereo
				for (int i=0; i<numSamples; i++) {
					int16_t l, r;
					PopFront(l, r);
					out[i*2+0] = (l>>7)+128;
					out[i*2+1] = (r>>7)+128; }}}}
#ifdef SHOW_TIMING
//vga::Color(255, { 0, 0, 0 });
#endif
	}


void PlayerAdapter::Refill() {
#ifdef SHOW_TIMING
vga::Color(255, { 0xc0, 0x40, 0x20 });
#endif
	// int numSamples = std::min(rw_.Available(), 128U);
	int numSamples = rw_.Available();
	if (numSamples > 0) {
		player_.Render(pbuf_, numSamples);
		for (int i=0; i<numSamples; ++i) {
			auto l = pbuf_[i*2+0];
			auto r = pbuf_[i*2+1];
			PushBack(l, r); }}
#ifdef SHOW_TIMING
vga::Color(255, { 0, 0, 0 });
#endif
	}


}  // namespace app
}  // namespace rqdq
