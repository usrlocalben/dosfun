#pragma once
#include <cstdint>

#include "pc_dma.hpp"
#include "pc_pic.hpp"

namespace rqdq {
namespace snd {

typedef void (*audioproc)(std::int16_t* dest, int numChannels, int numSamples, void *userPtr);

struct Ports {
	int reset;
	int read;
	int write;
	int poll;
	int ack16; };


class Blaster {
public:
	Blaster(int baseAddr, int irqNum, int dmaNum, int sampleRateInHz, int numChannels, int bufferSizeInSamples);

private:
	Blaster& operator=(const Blaster&);  // not copyable
	Blaster(const Blaster&);             // not copyable

	std::int16_t* GetUserBuffer() const;
	void SpinUntilReadyForWrite();
	void SpinUntilReadyForRead();
	void TX(std::uint8_t value);
	std::uint8_t RX();
	void RESET();
	bool SpinUntilReset();
	static void __interrupt isrJmp();
	void isr();
	void ACK();

public:
	bool IsGood() const;
	void AttachProc(audioproc userProc, void* userPtr);
	void DetachProc();

	~Blaster();

private:
	const Ports port_;
	pc::IRQLineRT irqLine_;
	const pc::Channel dma_;
	const int sampleRateInHz_;
	const int numChannels_;
	const int bufferSizeInSamples_;
	int userBuffer_;
	int playBuffer_;
	const pc::DMABuffer dmaBuffer_;
	bool good_;
	audioproc userProc_;
	void* userPtr_; };


}  // namespace snd
}  // namespace rqdq
