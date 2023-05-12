#pragma once

#include <cmath>
struct Vec4{
	Vec4(double _x, double _y, double _z, double _w) : x(_x), y(_y), z(_z), w(_w) {};
	union{
		struct{
			double x, y, z, w;
		};
		double values[4];
	};
};

struct Vec2{
	Vec2() : x(0), y(0) {};
	Vec2(double _x, double _y) : x(_x), y(_y) {};
	inline double length() const { return sqrt(x*x + y*y); }
	double x, y;
};

Vec2 operator+(const Vec2& a, Vec2& b);
Vec2 operator-(const Vec2& a, Vec2& b);
Vec2 operator*(const Vec2& a, double t);
double dot(const Vec2& a, const Vec2&b);
Vec2 normalize(const Vec2& a);

struct Vec3{
	Vec3() : x(0), y(0), z(0) {};
	Vec3(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {};
	inline double length() const { return sqrt(x*x + y*y + z*z); };
	inline Vec4 augmented() const { return Vec4(x, y, z, 1.0); };

	double x, y, z;
};

Vec3 operator+(const Vec3& a, Vec3& b);
Vec3 operator-(const Vec3& a, Vec3& b);
Vec3 operator*(const Vec3& a, double t);
double dot(const Vec3& a, const Vec3&b);
Vec3 normalize(const Vec3& a);
Vec3 cross(const Vec3& a, const Vec3& b);
Vec3 Vec3_rotate_z(const Vec3& v, double angle);
Vec3 Vec3_rotate_y(const Vec3& v, double angle);
Vec3 Vec3_rotate_x(const Vec3& v, double angle);

