#include "app_player_adapter.hpp"

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

	__asm__("sub %esp, 200\n\t" \
			"fsave (%esp)\n\t" \
			"finit");

#ifdef SHOW_TIMING
//vga::SetRGB(0, 0x20, 0x3f, 0x10);
#endif
	player_.Render(pbuf_, pbuf_+4096, numSamples);
#ifdef SHOW_TIMING
//vga::SetRGB(0, 0, 0, 0);
#endif
	if (fmt == 2) {
		// 16-bit signed PCM
		int16_t* out = static_cast<int16_t*>(out_);
		for (int i=0; i<numSamples; i++) {
			if (numChannels == 2) {
				out[i*2+0] = pbuf_[i]      * std::numeric_limits<int16_t>::max();
				out[i*2+1] = pbuf_[i+4096] * std::numeric_limits<int16_t>::max(); }
			else {
				out[i] = ((pbuf_[i]+pbuf_[i+4096])*0.5f) * std::numeric_limits<int16_t>::max(); }}}
	else {
		// 8-bit signed PCM
		uint8_t* out = static_cast<uint8_t*>(out_);
		for (int i=0; i<numSamples; i++) {
			if (numChannels == 2) {
				out[i*2+0] = (pbuf_[i]+1.0f)      * std::numeric_limits<int8_t>::max();
				out[i*2+1] = (pbuf_[i+4096]+1.0f) * std::numeric_limits<int8_t>::max(); }
			else {
				out[i] = (((pbuf_[i]+pbuf_[i+4096])*0.5f)+1.0f) * std::numeric_limits<int8_t>::max(); }}}

	__asm__("frstor (%esp)\n\t" \
			"add %esp, 200");
	}


}  // namespace app
}  // namespace rqdq
