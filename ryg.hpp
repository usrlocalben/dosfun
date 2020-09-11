#pragma once

namespace ryg {

typedef unsigned int uint;
typedef unsigned char uint8;

union FP32 {
    uint u;
    float f;
    struct {
        uint Mantissa : 23;
        uint Exponent : 8;
        uint Sign : 1;
        };
    };

extern float srgbtab[256];

extern uint fp32_to_srgb8_tab3[64];

inline
auto Linear(int c) -> float {
	return srgbtab[c]; }

inline
auto sRGB(float in) -> int {

	static const FP32 almostone = { 0x3f7fffff }; // 1-eps
	static const FP32 lutthresh = { 0x3b800000 }; // 2^(-8)
	static const FP32 linearsc = { 0x454c5d00 };
	static const FP32 float2int = { (127 + 23) << 23 };
	FP32 f;

	// Clamp to [0, 1-eps]; these two values map to 0 and 1, respectively.
	// The tests are carefully written so that NaNs map to 0, same as in the reference
	// implementation.
	if (!(in > 0.0f)) // written this way to catch NaNs
		in = 0.0f;
	if (in > almostone.f)
		in = almostone.f;

	// Check which region this value falls into
	f.f = in;
	if (f.f < lutthresh.f) {
		// linear region
		f.f *= linearsc.f;
		f.f += float2int.f; // use "magic value" to get float->int with rounding.
		return (uint8) (f.u & 255); }
	else {
		// non-linear region
		// Unpack bias, scale from table
		uint tab = fp32_to_srgb8_tab3[(f.u >> 20) & 63];
		uint bias = (tab >> 16) << 9;
		uint scale = tab & 0xffff;

		// Grab next-highest mantissa bits and perform linear interpolation
		uint t = (f.u >> 12) & 0xff;
		return (uint8) ((bias + scale*t) >> 16); }}

void Init();

}  // namespace ryg
