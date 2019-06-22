#pragma once
#include <cstdint>
#include <limits>

#include "mod.hpp"

namespace rqdq {
namespace app {

class PlayerAdapter {
public:
	PlayerAdapter(mod::Player& p);

	static void BlasterJmp(std::int16_t* out, int numChannels, int numSamples, void* self);

private:
	void BlasterProc(std::int16_t* out, int numChannels, int numSamples);

private:
	float pbuf_[4096*2];
	mod::Player& player_; };


}  // namespace app
}  // namespace rqdq
