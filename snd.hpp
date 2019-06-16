#pragma once
#include <cstdint>

#include "dma.hpp"
#include "pic.hpp"

namespace rqdq {

typedef void (*audioproc)(std::int16_t* dest, int num);

struct Ports {
	int reset;
	int read;
	int write;
	int poll;
	int ack16; };


class Blaster {
public:
	Blaster(int ioAddr, int irqNum, int dmaNum, int rate);

private:
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
	void AttachProc(audioproc value);
	void DetachProc();

	~Blaster();

private:
	const Ports port_;
	const PICInfo pic_;
	const DMAInfo dma_;
	const int sampleRateInHz_;
	int userBuffer_;
	int playBuffer_;
	const DMAPtr dmaMem_;
	bool good_;
	volatile audioproc audioProcPtr_; };


}  // namespace rqdq
