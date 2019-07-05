#pragma once

namespace rqdq {
namespace hw {

struct BlasterParams {
	BlasterParams(int a=-1, int b=-1, int c=-1, int d=-1)
		:ioAddr(a), irqNum(b), dmaLow(c), dmaHigh(d) {}
	int ioAddr;
	int irqNum;
	int dmaLow;
	int dmaHigh;

	int BestDMA() const {
		return dmaHigh != -1 ? dmaHigh : dmaLow; }};


class BlasterSerializer {
public:
	BlasterSerializer(const char* data=nullptr);
	BlasterParams Save() const {
		return params_; }
	bool IsValid();

private:
	void Deserialize(const char* text);
	void MaybeSaveCurrentField();

public:
	BlasterParams params_;
	int valid_;
	int curField_;
	int ax_;
	int base_; };


struct BlasterDetectResult {
	BlasterDetectResult(bool a, BlasterParams b) :found(a), value(b) {}
	bool found;
	BlasterParams value; };


BlasterDetectResult DetectBlaster();


}  // namespace hw
}  // namespace rqdq
