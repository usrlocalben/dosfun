#pragma once
#include <cstdint>
#include <limits>

#include "kb_tinymod.hpp"

namespace rqdq {
namespace app {

class PlayerAdapter {
public:
	PlayerAdapter(kb::ModPlayer& p);

	static void BlasterJmp(std::int16_t* out, int numChannels, int numSamples, void* self);

private:
	void BlasterProc(std::int16_t* out, int numChannels, int numSamples);

private:
	float pbuf_[4096*2];
	kb::ModPlayer& player_; };


}  // namespace app
}  // namespace rqdq
