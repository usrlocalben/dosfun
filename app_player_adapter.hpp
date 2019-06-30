#pragma once
#include <cstdint>
#include <limits>

#include "kb_tinymod.hpp"

namespace rqdq {
namespace app {

class PlayerAdapter {
public:
	PlayerAdapter(kb::ModPlayer& p) :player_(p) {}

	static void BlasterJmp(void* out, int fmt, int numChannels, int numSamples, void* self);

private:
	void BlasterProc(void* out, int fmt, int numChannels, int numSamples);

private:
	kb::ModPlayer& player_;
	float pbuf_[4096*2]; };


}  // namespace app
}  // namespace rqdq
