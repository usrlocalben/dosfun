#include "app_player_adapter.hpp"

#include <cstdint>
#include <limits>

#include "kb_tinymod.hpp"
#ifdef SHOW_TIMING
#include "vga_reg.hpp"
#endif

using std::uint8_t;
using std::int16_t;
using std::uint16_t;

namespace rqdq {
namespace app {

PlayerAdapter::PlayerAdapter(kb::ModPlayer& p)
	:player_(p) {}


void PlayerAdapter::BlasterJmp(int16_t* out, int numChannels, int numSamples, void* self) {
	static_cast<PlayerAdapter*>(self)->BlasterProc(out, numChannels, numSamples); }


void PlayerAdapter::BlasterProc(int16_t* out, int numChannels, int numSamples) {
#ifdef SHOW_TIMING
vga::SetRGB(0, 0x20, 0x3f, 0x10);
#endif
	player_.Render(pbuf_, pbuf_+4096, numSamples);
#ifdef SHOW_TIMING
vga::SetRGB(0, 0, 0, 0);
#endif
	for (int i=0; i<numSamples; i++) {
		if (numChannels == 2) {
			out[i*2+0] = pbuf_[i]      * std::numeric_limits<int16_t>::max();
			out[i*2+1] = pbuf_[i+4096] * std::numeric_limits<int16_t>::max(); }
		else {
			out[i] = ((pbuf_[i]+pbuf_[i+4096])*0.5f) * std::numeric_limits<int16_t>::max(); }}}


}  // namespace app
}  // namespace rqdq
