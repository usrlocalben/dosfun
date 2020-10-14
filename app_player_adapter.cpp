#include "app_player_adapter.hpp"

#include "kb_tinymod.hpp"
#ifdef SHOW_TIMING
#include "vga_reg.hpp"
#endif

#include <algorithm>
#include <cstdint>
#include <climits>

using std::int8_t;
using std::uint8_t;
using std::int16_t;
using std::uint16_t;

namespace rqdq {
namespace {

/**
 * Clamps a given integer x into the interval [a, b] with a <= b
 * Assuming INT_MIN <= x - a <= INT_MAX & INT_MIN <= r - b <= INT_MAX
 * _without branching_
 *
 * from las/hg on pouet:
 * https://www.pouet.net/topic.php?which=7800&page=1
 */
inline
auto ClampNB(int x, int a, int b) -> int {
	// r = max(x, a)
	int r = x - ((x - a) & ((x - a) >> (sizeof(int) * CHAR_BIT - 1)));
	// min(r, b)
	return b + ((r - b) & ((r - b) >> (sizeof(int) * CHAR_BIT - 1))); }

inline
auto SaturateS16(int x) -> int {
	return ClampNB(x, SHRT_MIN, SHRT_MAX); }

inline
auto S16toU8(int x) -> uint8_t {
	return (x>>8)+128; }

inline
auto Mono(int l, int r) -> int {
	return (l+r)>>1; }

constexpr int scratchSize = 2048;

alignas(16) int32_t scratch[scratchSize * 2];

constexpr int LEFT = 0;
constexpr int RIGHT = 1;

}
namespace app {

void PlayerAdapter::BlasterJmp(void* out, int fmt, int numChannels, int numSamples, void* self) {
	static_cast<PlayerAdapter*>(self)->BlasterProc(out, fmt, numChannels, numSamples); }
inline
void PlayerAdapter::BlasterProc(void* userBuffer, int fmt, int numChannels, int numSamples) {
#ifdef SHOW_TIMING
vga::Color(255, { 0xc0, 0x40, 0x20 });
#endif
	player_.Render(scratch, numSamples);
	if (fmt == 16) {
		// 16-bit signed PCM
		auto dest = static_cast<int16_t*>(userBuffer);
		if (numChannels == 1) {
			// mono
			for (int i=0; i<numSamples; ++i) {
				auto l = scratch[i];
				auto r = scratch[i+scratchSize];
				auto mono = Mono(l, r);
#ifdef MIX_SATURATE
				mono = SaturateS16(mono);
#endif
				dest[i] = mono; }}
		else {
			// stereo
			for (int i=0; i<numSamples; ++i) {
				auto l = scratch[i];
				auto r = scratch[i+scratchSize];
#ifdef MIX_SATURATE
				l = SaturateS16(l);
				r = SaturateS16(r);
#endif
				dest[i*2+LEFT]  = l;
				dest[i*2+RIGHT] = r; }}}
	else {
		// 8-bit unsigned PCM
		auto dest = static_cast<uint8_t*>(userBuffer);
		if (numChannels == 1) {
			// mono
			for (int i=0; i<numSamples; ++i) {
				auto l = scratch[i];
				auto r = scratch[i+scratchSize];
				auto mono = Mono(l, r);
#ifdef MIX_SATURATE
				mono = SaturateS16(mono);
#endif
				dest[i] = S16toU8(mono); }}
		else {
			// stereo
			for (int i=0; i<numSamples; i++) {
				auto l = scratch[i];
				auto r = scratch[i+scratchSize];
#ifdef MIX_SATURATE
				l = SaturateS16(l);
				r = SaturateS16(r);
#endif
				dest[i*2+0] = S16toU8(l);
				dest[i*2+1] = S16toU8(r); }}}

#ifdef SHOW_TIMING
vga::Color(255, { 0, 0, 0 });
#endif
	}


void PlayerAdapter::Refill() {
	return; }

/*
#ifdef SHOW_TIMING
vga::Color(255, { 0xc0, 0x40, 0x20 });
#endif
	// int numSamples = std::min(rw_.Available(), 256U);
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
*/


/*
	if (rw_.Size() >= numSamples) {
		if (fmt == 16) {
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
*/

}  // namespace app
}  // namespace rqdq
