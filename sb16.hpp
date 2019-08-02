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

public:
	Blaster(int baseAddr, int irqNum, int dmaNum, int sampleRateInHz, int numChannels, int bufferSizeInSamples);
	Blaster(Blaster&&) = default;
	Blaster& operator=(Blaster&&) = default;
	Blaster& operator=(const Blaster&) = delete;
	Blaster(const Blaster&) = delete;

	bool IsGood() const;
	void AttachProc(audioproc userProc, void* userPtr);
	void DetachProc();
	void Start();
	void Stop();

	~Blaster();

private:
	std::unique_ptr<impl> impl_; };


}  // namespace snd
}  // namespace rqdq
