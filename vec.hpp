#pragma once
#include <cstdint>
#include <cmath>

namespace rqdq {
namespace rml {

class IVec2 {
public:
	std::int32_t x, y;

	IVec2() = default;
	IVec2(const IVec2&) = default;
	IVec2& operator=(const IVec2&) = default;
	IVec2(IVec2&&) = default;
	IVec2& operator=(IVec2&&) = default;

	IVec2(int a) :x(a), y(a) {}
	IVec2(int a, int b) :x(a), y(b) {}

	bool operator==(IVec2 rhs) const { return x==rhs.x && y==rhs.y; }

	IVec2 operator+(IVec2 rhs) const { return { x+rhs.x, y+rhs.y }; }
	IVec2 operator-(IVec2 rhs) const { return { x-rhs.x, y-rhs.y }; }
	IVec2 operator*(IVec2 rhs) const { return { x*rhs.x, y*rhs.y }; }
	IVec2 operator/(IVec2 rhs) const { return { x/rhs.x, y/rhs.y }; }

	IVec2 operator+(std::int32_t rhs) const { return { x+rhs, y+rhs }; }
	IVec2 operator-(std::int32_t rhs) const { return { x-rhs, y-rhs }; }
	IVec2 operator*(std::int32_t rhs) const { return { x*rhs, y*rhs }; }
	IVec2 operator/(std::int32_t rhs) const { return { x/rhs, y/rhs }; }};


class IVec3 {
public:
	std::int32_t x, y, z;

	IVec3() = default;
	IVec3(const IVec3&) = default;
	IVec3& operator=(const IVec3&) = default;
	IVec3(IVec3&&) = default;
	IVec3& operator=(IVec3&&) = default;

	IVec3(int a) :x(a), y(a), z(a) {}
	IVec3(int a, int b, int c) :x(a), y(b), z(c) {}

	static IVec3 from_ulong(uint32_t value) {
		int x = value & 0xff;
		int y = (value>>8) & 0xff;
		int z = (value>>16) & 0xff;
		// int w = (value>>24) & 0xff;
		return { x, y, z }; } };


class Vec2 {
public:
	float x, y;

	Vec2() = default;
	Vec2(const Vec2&) = default;
	Vec2& operator=(const Vec2&) = default;
	Vec2(Vec2&&) = default;
	Vec2& operator=(Vec2&&) = default;

	Vec2(float a) :x(a), y(a) {}
	Vec2(float a, float b) :x(a), y(b) {}

	Vec2 operator+(float rhs) const { return { x+rhs, y+rhs }; }

	Vec2 operator+(Vec2 rhs) const { return { x+rhs.x, y+rhs.y }; }
	Vec2 operator-(Vec2 rhs) const { return { x-rhs.x, y-rhs.y }; }
	Vec2 operator*(Vec2 rhs) const { return { x*rhs.x, y*rhs.y }; }
	Vec2 operator/(Vec2 rhs) const { return { x/rhs.x, y/rhs.y }; }};


class Vec3 {
public:
	float x, y, z;

	Vec3() = default;
	Vec3(const Vec3&) = default;
	Vec3& operator=(const Vec3&) = default;
	Vec3(Vec3&&) = default;
	Vec3& operator=(Vec3&&) = default;

	Vec3(float a) :x(a), y(a), z(a) {}
	Vec3(float a, float b, float c) :x(a), y(b), z(c) {}

	Vec3 operator-(Vec3 rhs) const { return { x-rhs.x, y-rhs.y, z-rhs.z }; }
	Vec3 operator*(Vec3 rhs) const { return { x*rhs.x, y*rhs.y, z*rhs.z }; }};


inline auto itof(IVec2 a) -> Vec2 { return { float(a.x), float(a.y) }; }
inline auto itof(IVec3 a) -> Vec3 { return { float(a.x), float(a.y), float(a.z) }; }
inline auto ftoi(Vec2 a) -> IVec2 { return { int(a.x), int(a.y) }; }
inline auto ftoi(Vec3 a) -> IVec3 { return { int(a.x), int(a.y), int(a.z) }; }

inline auto operator+(Vec2 lhs, IVec2 rhs) -> Vec2 { return { lhs + itof(rhs) }; }
inline auto operator-(Vec2 lhs, IVec2 rhs) -> Vec2 { return { lhs - itof(rhs) }; }
inline auto operator*(Vec2 lhs, IVec2 rhs) -> Vec2 { return { lhs * itof(rhs) }; }
inline auto operator/(Vec2 lhs, IVec2 rhs) -> Vec2 { return { lhs / itof(rhs) }; }

inline auto Dot(Vec2 a, Vec2 b) -> float { return a.x*b.x + a.y*b.y; }
inline auto Dot(Vec3 a, Vec3 b) -> float { return a.x*b.x + a.y*b.y + a.z*b.z; }

inline auto Length(Vec2 a) -> float { return std::sqrt(Dot(a, a)); }
inline auto Length(Vec3 a) -> float { return std::sqrt(Dot(a, a)); }


}  // namespace rml
}  // namespace rqdq
