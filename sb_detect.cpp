#include "sb_detect.hpp"

#include <cstdlib>

namespace rqdq {
namespace {

constexpr auto IsValidPort(int a) -> bool {
	return a==0x220 || a==0x240 || a==0x260 || a==0x280; }

constexpr auto IsValidIRQ(int a) -> bool {
	return 3 <= a && a <= 15; }

constexpr auto IsValidDMA8(int a) -> bool {
	return 1 <= a && a <= 3; }

constexpr auto IsValidDMA16(int a) -> bool {
	return 5 <= a && a <= 7; }

auto hexToDec(char ch) -> int {
	if ('0' <= ch && ch <= '9') {
		return ch - '0'; }
	/*if ('a' <= ch && ch <= 'f') {
		return ch - 'a' + 10; }*/
	if ('A' <= ch && ch <= 'F') {
		return ch - 'A' + 10; }
	std::exit(1); }


auto toupper(char ch) -> char {
	if ('a' <= ch && ch <= 'z') {
		return 'A' + (ch - 'a'); }
	return ch; }


}  // namespace

namespace hw {

BlasterSerializer::BlasterSerializer(const char *data) :
	valid_(std::nullopt), curField_(0), ax_(0), base_(10) {
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
		params_.io = ax_; }
	else if (curField_ == 'I') {
		params_.irq = ax_; }
	else if (curField_ == 'D') {
		params_.dma8 = ax_; }
	else if (curField_ == 'H') {
		params_.dma16 = ax_; }
	curField_ = 0;
	ax_ = 0;
	base_ = 10; }


auto BlasterSerializer::IsValid() -> bool {
	if (valid_.has_value()) {
	// cout << "already validated, valid_==" << valid_ << "\n";
		return valid_.value(); }

	valid_ = false;
	if (!params_.io) {
		// cout << "no io addr\n";
		return false; }
	if (!IsValidPort(params_.io.value())) {
		// cout << "bad io\n";
		return false; }

	if (!params_.irq) {
		// cout << "no irq number\n";
		return false; }
	if (!IsValidIRQ(params_.irq.value())) {
		// cout << "bad irqnum\n";
		return false; }

	if (!params_.dma8 && !params_.dma16) {
		// must have at least one dma value
		// cout << "no dma values\n";
		return false; }
	if (params_.dma8 && !IsValidDMA8(params_.dma8.value())) {
		// cout << "dmalow is set but bad\n";
		return false; }
	if (params_.dma16 && !IsValidDMA16(params_.dma16.value())) {
		// cout << "dmahigh is set but bad\n";
		return false; }

	// cout << "seems valid to me\n";
	valid_ = true;
	return true; }


auto DetectBlaster() -> BlasterDetectResult {
	char* blasterStr = std::getenv("BLASTER");
	if (blasterStr != nullptr) {
// cout << "blaster is \"" << blasterStr << "\"\n";
		BlasterSerializer serializer(/*data=*/blasterStr);
		if (serializer.IsValid()) {
// cout << "serializer is valid\n";
			return BlasterDetectResult{ true, serializer.Save() }; }}

	return BlasterDetectResult{ false, BlasterParams() }; }


}  // namespace hw
}  // namespace rqdq
