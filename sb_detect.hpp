#pragma once
namespace rqdq {
namespace hw {

struct BlasterParams {
	int ioAddr;
	int irqNum;
	int dmaLow;
	int dmaHigh;

	BlasterParams(int a=-1, int b=-1, int c=-1, int d=-1) :
		ioAddr(a),irqNum(b), dmaLow(c), dmaHigh(d) {}

	auto BestDMA() const -> int {
		return dmaHigh != -1 ? dmaHigh : dmaLow; }};


class BlasterSerializer {
public:
	BlasterParams params_;
	int valid_;
	int curField_;
	int ax_;
	int base_;

public:
	BlasterSerializer(const char* data=nullptr);
	auto Save() const -> BlasterParams {
		return params_; }
	auto IsValid() -> bool;

private:
	void Deserialize(const char* text);
	void MaybeSaveCurrentField(); };


struct BlasterDetectResult {
	bool found;
	BlasterParams value; };

//	BlasterDetectResult(bool a, BlasterParams b) :
//		found(a), value(b) {} };


auto DetectBlaster() -> BlasterDetectResult;


}  // namespace hw
}  // namespace rqdq
