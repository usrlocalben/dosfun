#pragma once
#include "canvas.hpp"
#include "vga_pageflip.hpp"

#include <memory>

namespace rqdq {
namespace app {

class KefrensBars {
	class impl;
	std::unique_ptr<impl> impl_;

public:
	KefrensBars(int eh);
	~KefrensBars();
	KefrensBars(const KefrensBars&) = delete;
	auto operator=(const KefrensBars&) -> KefrensBars& = delete;

	void Draw(vga::DrawContext& dc, float T, int patternNum, int rowNum); };


}  // namespace app
}  // namespace rqdq
