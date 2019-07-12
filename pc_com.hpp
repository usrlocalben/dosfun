#pragma once
#include <string>

#include "algorithm.hpp"
#include "pc_pic.hpp"

namespace rqdq {
namespace pc {

const int kBufferSize = 2048;

const int FLOW_NONE = 0;
const int FLOW_RTSCTS = 1;
const int FLOW_DTRDSR = 2;
const int FLOW_XONXOFF = 3;


class SerialStream {
public:
	SerialStream(int ioBase, int irqNum, int baudRateInBitsPerSecond, int flowType);
	~SerialStream();

private:
	static void isrJmp();
	void isr();
	void OnDataReadyISR();
	void OnLineStatusChange();
	void OnModemStatusChange();
	void OnTXBufferEmpty();

	void Disable();
	void SetBaudRate(int bps);
	void SetWordSize(int bits);
	void SetParityMode(char mode);
	void SetStopSize(int bits);
	void SetFlowControl(int fc);
	void SetThreshold(int level);
		
	char InRBR();
	char InIER();
	char InIIR();
	char InLCR();
	char InMCR();
	char InLSR();
	char InMSR();
	int InDIV();

	void OutTHR(char value);
	void OutIER(char value);
	void OutFCR(char value);
	void OutLCR(char value);
	void OutMCR(char value);
	void OutDIV(int value);

	void SetDLAB(int num);
	void MaybeStartRXFlowing();

	bool RXBufferDanger() const;
	bool RXBufferSafe() const;

public:
	bool CanRead() const;
	bool CanWrite() const;
	int Read(char *buf, int len);
	int Write(const char *buf, int len);

private:
	int ioBase_;
	pc::IRQLineRT irqLine_;
	int bps_;

	char ier_{};
	char lsr_{};
	char msr_{};

	uint8_t rxBuf_[kBufferSize];
	uint8_t txBuf_[kBufferSize];
	alg::RingIndex<kBufferSize> rxRing_{};
	alg::RingIndex<kBufferSize> txRing_{};
	bool rxFlowing_{true};
	bool txFlowing_{true};
	int flowControlType_; };


// SerialStream MakeSerialStream(std::string descr, int baud);


}  // namespace pc
}  // namespace rqdq
