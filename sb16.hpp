#pragma once
#include <cstdint>

#include "pc_dma.hpp"
#include "pc_pic.hpp"

namespace rqdq {
namespace snd {

extern int spuriousIRQCnt;

typedef void (*audioproc)(void* dest, int fmt, int numChannels, int numSamples, void *userPtr);

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

	void SpeakerOn();
	void StartDMA();
	void StopDMA();
	void SpeakerOff();
	void SetSampleRate();

	void* GetUserBuffer() const;
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
	const pc::DMAChannel dma_;
	const int bits_;
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
