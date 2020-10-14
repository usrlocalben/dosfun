// this is kb/fr's tinymod.cpp without the pwm emulation
#pragma once
#include <array>
#include <cstdint>

namespace kb {

class Paula {
public:
	
	class Voice {
	public:
		int pos_;  // 16:16 fixed
		std::int8_t* samplePtr_;
		int sampleLen_;
		int loopLen_;
		int speed_;
		int period_;  // 124 .. 65535
		char volume_;  // 0 .. 64

		Voice();

		template <bool LOW_HZ>
		void Render(std::int32_t* buffer, int numSamples, int hz);

		void Trigger(std::int8_t* samplePtr, int sampleLen, int loopLen, int offs=0); };

	const int rate_;
	Voice voice_[4];
	int masterGain_;
	// float masterSeparation_;

public:
	Paula(int rate);
	void Render(std::int32_t* buf, int numSamples); };


class ModPlayer {
	Paula *paula_;

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
		char volume;
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

	int buf_[4096*2];

	void CalcTickRate(int bpm);
	void TrigNote(int ch, const Pattern::Event& e);
	void Reset();
	void Tick();

public:
	char name_[21];

	ModPlayer(Paula* paula, std::uint8_t* moddata);

	static
	void RenderJmp(void* param, std::int32_t* buf, int len);
	void Render(std::int32_t* buf, int numSamples);

	auto GetCurrentPos() const -> int {
		return curPos_; }
	auto GetCurrentRow() const -> int {
		return curRow_; } };


}  // namespace kb
