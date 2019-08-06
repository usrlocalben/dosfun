#pragma once
#include "alg_ringindex.hpp"
#include "pc_pic.hpp"

#include <string_view>

namespace rqdq {
namespace pc {

extern volatile int txCnt;

const int kBufferSize = 2048;

const int FLOW_NONE = 0;
const int FLOW_RTSCTS = 1;
const int FLOW_DTRDSR = 2;
const int FLOW_XONXOFF = 3;

class ComPort {
public:
	ComPort(int ioBase, int irqNum, int baudRateInBitsPerSecond, int flowType);
	~ComPort();

private:
	static void isrJmp();
	void isr();
	void isr_OnDataAvailable();
	void isr_OnLineStatusChanged();
	void isr_OnModemStatusChanged();
	void isr_OnTXBufferEmpty();

	void Disable();

	void MaybeStartRXFlowing();
	bool RXBufferDanger() const;
	bool RXBufferSafe() const;

	void SetBaudRate(int bps);
	void SetWordSize(int bits);
	void SetParityMode(char mode);
	void SetStopSize(int bits);
	void SetFlowControl(int fc);
	void SetThreshold(int level);
		
	void DLAB(int num);

	char RBR();
	             void THR(char value);
	char IER();  void IER(char value);
	char IIR();
	char LCR();  void LCR(char value);
	char MCR();  void MCR(char value);
	char LSR();
	char MSR();
	             void FCR(char value);
	int  DIV();  void DIV(int value);

public:
	bool DataAvailable() const {
		return rxRing_.Loaded(); }
	// int Read(char *buf, int len);
	std::string_view Peek(int limit) const;
	void Ack(std::string_view seg);

	bool CanWrite() const {
		return txFlowing_ && !rxRing_.Full(); }
	int Write(std::string_view buf);

private:
	int ioBase_;
	pc::IRQLineRT irqLine_;
	int bps_;

	char ier_{};
	char lsr_{};
	char msr_{};

	char rxBuf_[kBufferSize*2];
	char txBuf_[kBufferSize];
	alg::RingIndex<kBufferSize> rxRing_{};
	alg::RingIndex<kBufferSize> txRing_{};
	bool rxFlowing_{true};
	bool txFlowing_{true};
	int flowControlType_; };


// ComPort MakeComPort(std::string descr, int baud);


}  // namespace pc
}  // namespace rqdq
