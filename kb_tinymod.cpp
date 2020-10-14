// this is kb/fr's tinymod.cpp without the pwm emulation
#include "kb_tinymod.hpp"

#include "log.hpp"

#include <cassert>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::clamp;

namespace {

const float kPi = 3.1415926;
const float kPaulaPalHz = 3546895.0f;
const int kPALHz = 50;
const int kNumVoices = 4;

inline
void SwapEndian(uint16_t& v) {
	v = ((v&0xff)<<8)|(v>>8); }

int pTable[16][60];
int vibTable[3][15][64];
int basePTable[5*12+1] = { 0,
  1712,1616,1525,1440,1357,1281,1209,1141,1077,1017, 961, 907,    // c0-b0
   856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,    // c1-b1
   428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,    // c2-b2
   214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113,    // c3-b3
   107, 101,  95,  90,  85,  80,  76,  71,  67,  64,  60,  57, }; // c4-b4

#ifdef VOLTABLE
int16_t volTable[65][256];
#endif

struct inittables { inittables() {
	// calc ptable
	for (int ft=0; ft<16; ft++) {
		int rft = -((ft>=8)?ft-16:ft);
		float fac = std::pow(2.0f, float(rft)/(12.0f*16.0f));
		for (int i=0; i<60; i++) {
			pTable[ft][i] = int(float(basePTable[i])*fac+0.5f); }}

	// calc vibtable
	for (int ampl=0; ampl<15; ampl++) {
		float scale = ampl + 1.5f;
		float shift = 0;
		for (int x=0; x<64; x++) {
			vibTable[0][ampl][x] = int(scale * std::sin(x*kPi/32.0f)+shift);
			vibTable[1][ampl][x] = int(scale*((63-x)/31.5f-1.0f)+shift);
			vibTable[2][ampl][x] = int(scale*((x<32)?1:-1)+shift); }}

#ifdef VOLTABLE
	for (int vol=0; vol<=64; ++vol) {
		for (int n=0; n<=255; ++n) {
			volTable[vol][n] = int(int8_t(n))*vol<<2; }}
#endif

	}} inittables;


}  // namespace

namespace kb {

Paula::Voice::Voice() :
	pos_(0),
	samplePtr_(nullptr),
	sampleLen_(0),
	loopLen_(1<<10),
	period_(65535),
	speed_(0),
	volume_(0) {}


template <bool LOW_HZ>
void Paula::Voice::Render(int32_t* buffer, int numSamples, int hz) {
	if (samplePtr_ == nullptr) {
		return; }
	if (volume_ == 0) {
		return; }

	int volume = volume_;
#ifndef VOLTABLE
	volume <<= 2;  // also converts sample to 16-bit
#endif

	int speed = uint32_t(3546895)*1024 / uint32_t(period_ * hz);

	for (int i=0; i<numSamples; i++) {
		int32_t v;
#ifdef VOLTABLE
		v = volTable[volume][uint8_t(samplePtr_[pos_>>10])];
#else
		v = samplePtr_[pos_>>10];
		v *= volume;  // v is now 16-bits
#endif
		*buffer += v;
		buffer++;

		pos_ += speed;
		if (pos_ >= sampleLen_) {
			// kb's impl used looplen==1 to disable
			// loop, which avoids branching here, but
			// it doesn't work if speed is sufficiently
			// large, which is the case when the samplerate
			// is sufficiently low, e.g. 8khz.
			if constexpr (LOW_HZ) {
				pos_ -= loopLen_ == 1024 ? speed : loopLen_; }
			else {
				pos_ -= loopLen_; }}}}


void Paula::Voice::Trigger(int8_t* samplePtr, int sampleLen, int loopLen, int offs) {
	samplePtr_ = samplePtr;
	sampleLen_ = sampleLen << 10;
	loopLen_ = loopLen << 10;
	pos_ = std::min(offs, sampleLen-1) << 10; }


Paula::Paula(int rate) : rate_(rate), masterGain_(0x100) {}


void Paula::Render(int32_t* buf, int numSamples) {
	// const float pan = 0.5f + 0.5f * masterSeparation_;
	// const float vm0 = masterGain_ * sqrt(pan);
	// const float vm1 = masterGain_ * sqrt(1-pan);

	for (int i=0; i<numSamples; ++i) {
		buf[i] = 0; }
	for (int i=0; i<numSamples; ++i) {
		buf[i+2048] = 0; }
	for (int vi=0; vi<kNumVoices; ++vi) {
		// 0,3=left, 1,2=right
		int offset = static_cast<int>(vi==1||vi==2) * 2048;
		if (rate_ < 20000) {
			voice_[vi].Render<true>(buf+offset, numSamples, rate_); }
		else {
			voice_[vi].Render<false>(buf+offset, numSamples, rate_); }}
	if (masterGain_ < 0x100) {
		for (int i=0; i<numSamples; ++i) {
			buf[i] = buf[i] * masterGain_ >> 8; }
		for (int i=0; i<numSamples; ++i) {
			buf[i+2048] = buf[i+2048] * masterGain_ >> 8; }}}


void ModPlayer::Sample::Prepare() {
	SwapEndian(length);
	SwapEndian(loopStart);
	SwapEndian(loopLen);
	fineTune &= 0x0f;
	if (fineTune >= 8) {
		fineTune -= 16; }}


ModPlayer::Pattern::Pattern() {
	std::memset(this, 0, sizeof(Pattern)); }


void ModPlayer::Pattern::Load(uint8_t* ptr) {
	for (int row=0; row<64; row++) {
		for (int ch=0; ch<kNumVoices; ch++) {
			Event& e = events_[row][ch];
			e.sampleNum = (ptr[0] & 0xf0) | (ptr[2]>>4);
			e.fx        = ptr[2] & 0x0f;
			e.fxParam   = ptr[3];

			e.note = 0;
			int period = (int(ptr[0]&0x0f)<<8) | ptr[1];
			int bestD = std::abs(period - basePTable[0]);
			if (period) {
				for (int i=1; i<=60; i++) {
					int d = std::abs(period-basePTable[i]);
					if (d < bestD) {
						bestD = d;
						e.note = i; }}}
			ptr += 4; }}}


ModPlayer::Chan::Chan() {
	std::memset(this, 0, sizeof(Chan)); }


int ModPlayer::Chan::GetPeriod(int offs, int fineOffs) {
	int ft = fineTune + fineOffs;
	while (ft >  7) { offs++; ft -= 16; }
	while (ft < -8) { offs--; ft += 16; }
	return note ? (pTable[ft&0x0f][clamp(note+offs-1, 0, 59)]) : 0; }


void ModPlayer::Chan::SetPeriod(int offs, int fineOffs) {
	if (note) {
		period = GetPeriod(offs, fineOffs); }}


void ModPlayer::CalcTickRate(int bpm) {
	tickRate_ = (125*paula_->rate_)/(bpm*kPALHz); }


// float ModPlayer::GetTickDurationInSeconds() {
//	// bpm*fps/125
//	return kOutputFreqInHz / tickRate_; }


void ModPlayer::TrigNote(int ch, const Pattern::Event& e) {
	Chan& c = chans_[ch];
	Paula::Voice& v = paula_->voice_[ch];
	const Sample& s = sampleDB_[c.sampleNum];
	int offset = 0;

	if (e.fx == 9) {
		offset = c.fxBuf[9] << 8; }
	if (e.fx != 3 && e.fx != 5) {
		c.SetPeriod();
		if (s.loopLen > 1) {
			v.Trigger(pcmPtr_[c.sampleNum], 2*(s.loopStart+s.loopLen), 2*s.loopLen, offset); }
		else {
			v.Trigger(pcmPtr_[c.sampleNum], v.sampleLen_=2*s.length, 1, offset); }
		if (!c.vibRetr) {
			c.vibPos = 0; }
		if (!c.tremRetr) {
			c.tremPos = 0; }}}


void ModPlayer::Reset() {
	CalcTickRate(125);
	speed_ = 6;
	trCounter_ = 0;
	curTick_ = 0;
	curRow_ = 0;
	curPos_ = 0;
	delay_ = 0; }


void ModPlayer::Tick() {
	const Pattern& p = patterns_[patternList_[curPos_]];
	const Pattern::Event* re = p.events_[curRow_];
	for (int ch=0; ch<kNumVoices; ch++) {
		const Pattern::Event& e = re[ch];
		Paula::Voice& v = paula_->voice_[ch];
		Chan& c = chans_[ch];
		const int fxpl = e.fxParam & 0x0f;
		int tremVol = 0;
		if (curTick_ == 0) {
			if (e.sampleNum) {
				c.sampleNum = e.sampleNum;
				c.fineTune = sampleDB_[c.sampleNum].fineTune;
				c.volume = sampleDB_[c.sampleNum].volume; }

			if (e.fxParam) {
				c.fxBuf[e.fx] = e.fxParam; }

			if (e.note && (e.fx!=14 || ((e.fxParam>>4)!=13))) {
				c.note = e.note;
				TrigNote(ch, e); }

			switch (e.fx) {
			case 4:  // vibrato
			case 6:
				if (c.fxBuf[4]&0x0f) c.vibAmpl = c.fxBuf[4]&0x0f;
				if (c.fxBuf[4]&0xf0) c.vibSpeed = c.fxBuf[4]>>4;
				c.SetPeriod(0, vibTable[c.vibWave][(c.vibAmpl)-1][c.vibPos]);
				break;
			case 7:  // tremolo
				if (c.fxBuf[7]&0x0f) c.tremAmpl = c.fxBuf[7]&0x0f;
				if (c.fxBuf[7]&0xf0) c.tremSpeed = c.fxBuf[7]>>4;
				tremVol = vibTable[c.tremWave][(c.tremAmpl)-1][c.tremPos];
				break;
			case 12:  // set vol
				c.volume = clamp(e.fxParam, 0, 64);
				break;
			case 14:  // special
				if (fxpl) {
					c.fxBuf14[e.fxParam>>4] = fxpl; }
				switch (e.fxParam >> 4) {
				case 0:  // set filter
					break;
				case 1:  // set fineslide up
					c.period = std::max(113, c.period - c.fxBuf14[1]);
					break;
				case 2:  // slide down
					c.period = std::max(856, c.period + c.fxBuf14[2]);
					break;
				case 3:  // set glissando sucks!
					break;
				case 4:  // set vib waveform
					c.vibWave = fxpl & 3;
					if (c.vibWave == 3) c.vibWave = 0;
					c.vibRetr = fxpl & 4;
					break;
				case 5:  // set finetune
					c.fineTune = fxpl;
					if (c.fineTune >= 8) c.fineTune -= 16;
					break;
				case 7:  // set tremolo
					c.tremWave = fxpl & 3;
					if (c.tremWave == 3) c.tremWave = 0;
					c.tremRetr = fxpl & 4;
					break;
				case 9:  // retrigger
					if (c.fxBuf14[9] && !e.note) {
						TrigNote(ch, e); }
					c.retrigCount = 0;
					break;
				case 10:  // fine volslide up
					c.volume = std::min(c.volume + c.fxBuf14[10], 64);
					break;
				case 11:  // fine volslide down
					c.volume = std::max(c.volume - c.fxBuf14[11], 0);
					break;
				case 14:  // delay pattern
					delay_ = c.fxBuf14[14];
					break;
				case 15:  // invert loop (WTF)
					break; }
				break;
			case 15:  // set speed
				if (e.fxParam) {
					if (e.fxParam <= 32) {
						speed_ = e.fxParam; }
					else {
						CalcTickRate(e.fxParam); }}
				break; }}

		else {
			switch (e.fx) {
			case 0:  // arpeggio
				if (e.fxParam) {
					int no = 0;
					switch (curTick_ % 3) {
					case 1: no = e.fxParam >> 4; break;
					case 2: no = e.fxParam & 0x0f; break; }
					c.SetPeriod(no); }
				break;
			case 1:  // slide up
				c.period = std::max(113, c.period - c.fxBuf[1]);
				break;
			case 2:  // slide down
				c.period = std::min(856, c.period + c.fxBuf[2]);
				break;
			case 5:  // slide plus volslide
				if (c.fxBuf[5] & 0xf0) {
					c.volume = std::min(c.volume + (c.fxBuf[5]>>4), 0x40); }
				else {
					c.volume = std::max(c.volume - (c.fxBuf[5]&0x0f), 0); }
				// no break!
			case 3:  // slide to note
				{int np = c.GetPeriod();
				if (c.period > np) {
					c.period = std::max(c.period - c.fxBuf[3], np); }
				else {
					c.period = std::min(c.period + c.fxBuf[3], np); }}
				break;
			case 6:  // vibrato plus volslide
				if (c.fxBuf[6] & 0xf0) {
					c.volume = std::min(c.volume + (c.fxBuf[6]>>4), 0x40); }
				else {
					c.volume = std::max(c.volume - (c.fxBuf[6]&0x0f), 0); }
				// no break!
			case 4:  // vibrato ???
				c.SetPeriod(0, vibTable[c.vibWave][c.vibAmpl-1][c.vibPos]);
				c.vibPos = (c.vibPos+c.vibSpeed)&0x3f;
				break;
			case 7:  // tremolo ???
				tremVol = vibTable[c.tremWave][c.tremAmpl-1][c.tremPos];
				c.tremPos = (c.tremPos+c.tremSpeed)&0x3f;
				break;
			case 10:  // volslide
				if (c.fxBuf[10] & 0xf0) {
					c.volume = std::min(c.volume+(c.fxBuf[10]>>4), 0x40); }
				else {
					c.volume = std::max(c.volume-(c.fxBuf[10]&0x0f), 0); }
				break;
			case 11:  // pos jump
				if (curTick_ == speed_-1) {
					curRow_ = -1;
					curPos_ = e.fxParam; }
				break;
			case 13:  // pattern break
				if (curTick_ == speed_-1) {
					curPos_++;
					curRow_ = (10*(e.fxParam>>4)+(e.fxParam&0x0f))-1; }
				break;
			case 14:  // special
				switch (e.fxParam >> 4) {
				case 6:  // loop pattern
					if (!fxpl) {
						// loop start
						c.loopStart = curRow_; }
					else if (curTick_ == speed_-1) {
						if (c.loopCount < fxpl) {
							curRow_ = c.loopStart - 1;
							c.loopCount++; }
						else {
							c.loopCount = 0; }}
					break;
				case 9:  // retrigger
					if (++c.retrigCount == c.fxBuf14[9]) {
						c.retrigCount = 0;
						TrigNote(ch, e); }
					break;
				case 12:  // cut
					if (curTick_ == c.fxBuf14[12]) {
						c.volume = 0; }
					break;
				case 13:  // delay
					if (curTick_ == c.fxBuf14[13]) {
						TrigNote(ch, e); }
					break; }
				break; }}
		v.volume_ = clamp(c.volume + tremVol, 0, 64);
		v.period_ = c.period; }

	curTick_++;
	if (curTick_ >= speed_*(delay_+1)) {
		curTick_ = 0;
		curRow_++;
		delay_ = 0; }
	if (curRow_ >= 64) {
		curRow_ = 0;
		curPos_++; }
	if (curPos_ >= positionCount_) {
		curPos_ = 0; }}


ModPlayer::ModPlayer(Paula* p, uint8_t* moddata) :paula_(p) {

			
	// "load" the mod
	std::memcpy(name_, moddata, 20);
	name_[20] = 0;
	moddata += 20;

	sampleCount_ = 16;
	channelCount_ = 4;
	sampleDB_ = (Sample*)(moddata-sizeof(Sample));
	moddata += 15*sizeof(Sample);

	uint32_t& tag = *(uint32_t*)(moddata+130+16*sizeof(Sample));

	switch (tag) {
	case '.K.M':
	case '4TLF':
	case '!K!M':
		sampleCount_ = 32;
		break; }
	if (sampleCount_ > 16) {
		moddata += (sampleCount_-16)*sizeof(Sample); }
	for (int i=1; i<sampleCount_; i++) {
		sampleDB_[i].Prepare(); }

	positionCount_ = *moddata; moddata++;

	moddata++;  // skip unused byte

	std::memcpy(patternList_, moddata, 128);  moddata += 128;

	if (sampleCount_ > 16) {
		moddata += 4; }  // skip tag

	patternCount_ = 0;
	for (int i=0; i<128; i++) {
		patternCount_ = clamp(patternCount_, patternList_[i]+1, 128); }

	for (int i=0; i<patternCount_; i++) {
		patterns_[i].Load(moddata);
		moddata += 1024; }

	std::memset(pcmPtr_, 0, sizeof(pcmPtr_));
	for (int i=1; i<sampleCount_; i++) {
		pcmPtr_[i] = (int8_t*)moddata;
		moddata += 2 * sampleDB_[i].length; }

	Reset(); }


void ModPlayer::RenderJmp(void* param, int32_t* buf, int len) {
	((ModPlayer*)param)->Render(buf, len); }
void ModPlayer::Render(int32_t* buf, int numSamples) {
	while (numSamples) {
		int todo = std::min(numSamples, trCounter_);
		if (todo) {
			paula_->Render(buf, todo);
			buf += todo;
			numSamples -= todo;
			trCounter_ -= todo; }
		else {
			Tick();
			trCounter_ = tickRate_; }}}


}  // namespace kb
