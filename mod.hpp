// this is kb/fr's tinymod.cpp without the pwm emulation
#pragma once
#include <cstdint>

namespace rqdq {

class Paula {
public:
	
	class Voice {
	public:
		float pos_;
		std::int8_t* samplePtr_;
		int sampleLen_;
		int loopLen_;
		int period_;  // 124 .. 65535
		int volume_;  // 0 .. 64

		Voice();
		void Render(float* buffer, int numSamples);
		void Trigger(std::int8_t* samplePtr, int sampleLen, int loopLen, int offs=0); };

	Voice voice_[4];
	float out_[4096];  // *2];  // left, right
	float masterGain_;
	// float masterSeparation_;

public:
	Paula();
	void Render(float *outBuf, int numSamples); };


class ModPlayer {
	Paula *paula_;
	static int basePTable[5*12 + 1];
	static int pTable[16][60];
	static int vibTable[3][15][64];
#pragma pack(1)
	struct Sample {
		char name[22];
		std::uint16_t length;
		std::int8_t fineTune;
		std::uint8_t volume;
		std::uint16_t loopStart;
		std::uint16_t loopLen;

		void Prepare();};
#pragma pack()
	struct Pattern {
		struct Event {
			int sampleNum;
			int note;
			int fx;
			int fxParam; };
		Event events_[64][4];
		Pattern();
		void Load(std::uint8_t* ptr); };

	Sample *sampleDB_;
	std::int8_t* pcmPtr_[32];
	int sampleCount_;
	int channelCount_;

	std::uint8_t patternList_[128];
	int positionCount_;
	int patternCount_;

	Pattern patterns_[128];

	struct Chan {
		int note;
		int period;
		int sampleNum;
		int fineTune;
		int volume;
		int fxBuf[16];
		int fxBuf14[16];
		int loopStart;
		int loopCount;
		int retrigCount;
		int vibWave;
		int vibRetr;  // xxx ????
		int vibPos;
		int vibAmpl;
		int vibSpeed;
		int tremWave;
		int tremRetr;
		int tremPos;
		int tremAmpl;
		int tremSpeed;

		Chan();
		int GetPeriod(int offs=0, int fineOffs=0);
		void SetPeriod(int offs=0, int fineOffs=0); };

	Chan chans_[4];

	int speed_;
	int tickRate_;
	int trCounter_;  // xxx ???

	int curTick_;
	int curRow_;
	int curPos_;
	int delay_;

	void CalcTickRate(int bpm);
	void TrigNote(int ch, const Pattern::Event& e);
	void Reset();
	void Tick();

public:
	char name_[21];

	ModPlayer(Paula* paula, std::uint8_t* moddata);
	void Render(float* buf, int numSamples);
	int GetCurrentPos() const {
		return curPos_; }
	static void RenderJmp(void* param, float* buf, int len); };


}  // namespace rqdq
