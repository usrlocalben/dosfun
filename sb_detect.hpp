#pragma once
#include <optional>

namespace rqdq {
namespace hw {

struct BlasterParams {
	std::optional<int> io{};
	std::optional<int> irq{};
	std::optional<int> dma8{};
	std::optional<int> dma16{}; };


class BlasterSerializer {

	BlasterParams params_{};
	std::optional<bool> valid_{};
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

auto DetectBlaster() -> BlasterDetectResult;


}  // namespace hw
}  // namespace rqdq
