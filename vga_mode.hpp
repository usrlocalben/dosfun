#pragma once

namespace rqdq {
namespace vga {

class ModeSetter {
	const int oldMode_;

	int width_{0};
	int height_{0};
	int stride_{0};
	bool linear_{true};

public:
	ModeSetter();

	// non-copyable
	ModeSetter(const ModeSetter&) = delete;
	auto operator=(const ModeSetter&) -> ModeSetter& = delete;

	~ModeSetter();

	void Set(int width, int height, bool linear); };


}  // namespace vga
}  // namespace rqdq
