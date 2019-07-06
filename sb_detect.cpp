#include "sb_detect.hpp"

#include <cstdlib>

// using std::cout;

namespace rqdq {
namespace {

int hexToDec(char ch) {
	if ('0' <= ch && ch <= '9') {
		return ch - '0'; }
	/*if ('a' <= ch && ch <= 'f') {
		return ch - 'a' + 10; }*/
	if ('A' <= ch && ch <= 'F') {
		return ch - 'A' + 10; }
	std::exit(1); }


char toupper(char ch) {
	if ('a' <= ch && ch <= 'z') {
		return 'A' + (ch - 'a'); }
	return ch; }


}  // namespace

namespace hw {

BlasterSerializer::BlasterSerializer(const char *data)
	:params_(-1,-1,-1,-1), valid_(-1), curField_(0), ax_(0), base_(10) {
	if (data != nullptr) {
		Deserialize(data); }}


void BlasterSerializer::Deserialize(const char* text) {
	for (; *text!=0; text++) {
		const char ch = toupper(*text);
		if (ch == 'A') {
			curField_ = ch;
			base_ = 16; }
		else if (ch == 'I') {
			curField_ = ch;
			base_ = 10; }
		else if (ch == 'D') {
			curField_ = ch;
			base_ = 10; }
		else if (ch == 'H') {
			curField_ = ch;
			base_ = 10; }
		else if (ch == ' ') {
			MaybeSaveCurrentField(); }
		else {
			if (base_ == 10) {
				ax_ = ax_*10 + ch - '0'; }
			else {
				ax_ = ax_*16 + hexToDec(ch); }}}
	MaybeSaveCurrentField(); }


void BlasterSerializer::MaybeSaveCurrentField() {
	if (curField_ == 'A') {
		params_.ioAddr = ax_; }
	else if (curField_ == 'I') {
		params_.irqNum = ax_; }
	else if (curField_ == 'D') {
		params_.dmaLow = ax_; }
	else if (curField_ == 'H') {
		params_.dmaHigh = ax_; }
	curField_ = 0;
	ax_ = 0;
	base_ = 10; }


bool BlasterSerializer::IsValid() {
	if (valid_ != -1) {
// cout << "already validated, valid_==" << valid_ << "\n";
		return valid_ == 1; }

	const int ioAddr = params_.ioAddr;
	const int irqNum = params_.irqNum;
	const int dmaLow = params_.dmaLow;
	const int dmaHigh = params_.dmaHigh;

	if (!(0x200 <= ioAddr && ioAddr < 0x300)) {
		// not a reasonable looking io base addr
// cout << "bad ioaddr\n";
		valid_ = 0; return false; }
	if (!(3 <= irqNum && irqNum <= 15)) {
		// not a reasonable looking irq num
// cout << "bad irqnum\n";
		valid_ = 0; return false; }
	if (!(dmaLow != -1 || dmaHigh != -1)) {
		// must have at least one dma value
// cout << "no dma values\n";
		valid_ = 0; return false; }
	if (dmaLow != -1) {
		if (!(1 <= dmaLow && dmaLow <= 3)) {
// cout << "dmalow is set but bad\n";
			valid_ = 0; return false; }}
	if (dmaHigh != -1) {
		if (!(5 <= dmaHigh && dmaHigh <= 7)) {
// cout << "dmahigh is set but bad\n";
			valid_ = 0; return false; }}
// cout << "seems valid to me\n";
	valid_ = 1;
	return true; }


BlasterDetectResult DetectBlaster() {
	char* blasterStr = std::getenv("BLASTER");
	if (blasterStr != nullptr) {
// cout << "blaster is \"" << blasterStr << "\"\n";
		BlasterSerializer serializer(/*data=*/blasterStr);
		if (serializer.IsValid()) {
// cout << "serializer is valid\n";
			return BlasterDetectResult( true, serializer.Save() ); }}

	return BlasterDetectResult( false, BlasterParams() ); }


}  // namespace hw
}  // namespace rqdq
