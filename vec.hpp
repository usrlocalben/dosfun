#pragma once
#include <cstdint>
#include <cmath>

namespace rqdq {
namespace rml {

class IVec2 {
public:
	IVec2() = default;
	IVec2(const IVec2&) = default;
	IVec2& operator=(const IVec2&) = default;
	IVec2(IVec2&&) = default;
	IVec2& operator=(IVec2&&) = default;

	bool operator==(IVec2 rhs) const { return x==rhs.x && y==rhs.y; }

	IVec2(int a) :x(a), y(a) {}
	IVec2(int a, int b) :x(a), y(b) {}

	std::int32_t x, y; };


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


class Vec2 {
public:
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
	Vec2 operator/(Vec2 rhs) const { return { x/rhs.x, y/rhs.y }; }

	float x, y; };

class Vec3 {
public:
	Vec3() = default;
	Vec3(const Vec3&) = default;
	Vec3& operator=(const Vec3&) = default;
	Vec3(Vec3&&) = default;
	Vec3& operator=(Vec3&&) = default;

	Vec3(float a) :x(a), y(a), z(a) {}
	Vec3(float a, float b, float c) :x(a), y(b), z(c) {}

	Vec3 operator-(Vec3 rhs) const { return { x-rhs.x, y-rhs.y, z-rhs.z }; }
	Vec3 operator*(Vec3 rhs) const { return { x*rhs.x, y*rhs.y, z*rhs.z }; }

	float x, y, z; };


inline Vec2 itof(IVec2 a) { return Vec2{ float(a.x), float(a.y) }; }
inline Vec3 itof(IVec3 a) { return Vec3{ float(a.x), float(a.y), float(a.z) }; }
inline IVec2 ftoi(Vec2 a) { return IVec2{ int(a.x), int(a.y) }; }
inline IVec3 ftoi(Vec3 a) { return IVec3{ int(a.x), int(a.y), int(a.z) }; }

inline Vec2 operator+(Vec2 lhs, IVec2 rhs) { return { lhs + itof(rhs) }; }
inline Vec2 operator-(Vec2 lhs, IVec2 rhs) { return { lhs - itof(rhs) }; }
inline Vec2 operator*(Vec2 lhs, IVec2 rhs) { return { lhs * itof(rhs) }; }
inline Vec2 operator/(Vec2 lhs, IVec2 rhs) { return { lhs / itof(rhs) }; }

inline float Dot(Vec2 a, Vec2 b) { return a.x*b.x + a.y*b.y; }
inline float Dot(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

inline float Length(Vec2 a) { return std::sqrt(Dot(a, a)); }
inline float Length(Vec3 a) { return std::sqrt(Dot(a, a)); }


}  // namespace rml
}  // namespace rqdq
