#pragma once
#include "pc_dma.hpp"
#include "pc_pic.hpp"

#include <cstdint>
#include <memory>

namespace rqdq {
namespace snd {

extern int spuriousIRQCnt;

typedef void (*audioproc)(void* dest, int fmt, int numChannels, int numSamples, void *userPtr);

class Blaster {
	class impl;

	std::unique_ptr<impl> impl_;

public:
	Blaster(int baseAddr, int irqNum, int dmaNum, int sampleRateInHz, int numChannels, int bufferSizeInSamples);
	Blaster(Blaster&&) = default;
	auto operator=(Blaster&&) -> Blaster& = default;
	Blaster(const Blaster&) = delete;
	auto operator=(const Blaster&) -> Blaster& = delete;

	auto IsGood() const -> bool;
	void AttachProc(audioproc userProc, void* userPtr);
	void DetachProc();
	void Start();
	void Stop();

	~Blaster(); };


}  // namespace snd
}  // namespace rqdq
