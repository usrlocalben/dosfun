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
					out[i*2+0] = l;
					out[i*2+1] = r; }
				else {
					out[i] = (l+r)>>1; }}}
		else {
			// 8-bit signed PCM
			uint8_t* out = static_cast<uint8_t*>(out_);
			for (int i=0; i<numSamples; i++) {
				int l, r;
				PopFront(l, r);
				if (numChannels == 2) {
					out[i*2+0] = (l+32767)>>8;
					out[i*2+1] = (r+32767)>>8; }
				else {
					out[i] = (l+r+65536)>>9; }}}}
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
			int l = pbuf_[i] * 32767.0;
			int r = pbuf_[i+4096] * 32767.0;
			PushBack(l, r); }}
#ifdef SHOW_TIMING
vga::SetRGB(0, 0, 0, 0);
#endif
	}


}  // namespace app
}  // namespace rqdq
