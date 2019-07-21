#pragma once
#include <cstdint>

namespace rqdq {
namespace rml {

class IVec3 {
public:
	IVec3() = default;
	IVec3(const IVec3&) = default;
	IVec3& operator=(const IVec3&) = default;
	IVec3(IVec3&&) = default;
	IVec3& operator=(IVec3&&) = default;

	IVec3(int a) :x(a), y(a), z(a) {}
	IVec3(int a, int b, int c) :x(a), y(b), z(c) {}

	std::int32_t x, y, z; };


}  // namespace rml
}  // namespace rqdq
