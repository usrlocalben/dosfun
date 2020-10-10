#pragma once
#include "pc_dma.hpp"
#include "pc_pic.hpp"

#include <cstdint>
#include <memory>

namespace rqdq {
namespace snd {

extern int spuriousIRQCnt;

typedef void (*audioproc)(void* dest, int fmt, int numChannels, int numSamples, void *userPtr);

struct BlasterConfig {
	int io;
	int irq;
	int dma8;
	int dma16; };

class Blaster {
	class impl;

	std::unique_ptr<impl> impl_;

public:
	Blaster(BlasterConfig config, int sampleRateInHz, int numChannels, int bufferSizeInSamples);
	Blaster(Blaster&&) = default;
	auto operator=(Blaster&&) -> Blaster& = default;
	Blaster(const Blaster&) = delete;
	auto operator=(const Blaster&) -> Blaster& = delete;

	auto IsGood() const -> bool;
	void AttachProc(audioproc userProc, void* userPtr);
	void DetachProc();
	void Start();
	void Stop();

	auto Rate() const -> int;

	~Blaster(); };


}  // namespace snd
}  // namespace rqdq
