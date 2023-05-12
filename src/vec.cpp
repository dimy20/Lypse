#include "vec.h"

inline Vec2 operator+(const Vec2& a, Vec2& b){ return Vec2(a.x + b.x, a.y + b.y); };
inline Vec2 operator-(const Vec2& a, Vec2& b){ return Vec2(a.x + b.x, a.y + b.y); };
inline Vec2 operator*(const Vec2& a, double t){ return Vec2(a.x * t, a.y * t); };
inline double dot(const Vec2& a, const Vec2& b){ return a.x*b.x + a.y*b.y; };
inline Vec2 normalize(const Vec2& a){ double len = a.length(); return Vec2(a.x /len, a.y / len); }

inline Vec3 operator+(const Vec3& a, Vec3& b){ return Vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline Vec3 operator-(const Vec3& a, Vec3& b){ return Vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline Vec3 operator*(const Vec3& a, double t){ return Vec3(a.x * t, a.y * t, a.z * t); }
inline double dot(const Vec3& a, const Vec3&b){ return a.x*b.x + a.y*b.y + a.z*b.z; }

inline Vec3 normalize(const Vec3& a){
	double len = a.length();
	return Vec3(a.x / len, a.y / len, a.z / len);
}

inline Vec3 cross(const Vec3& a, const Vec3& b){
	double x, y, z;
	x = a.y*b.z - a.z*b.y;
	y = a.z*b.x - a.x*b.z;
	z = a.x*b.y - a.y*b.x;
	return Vec3(x, y, z);
};

inline Vec3 Vec3_rotate_z(const Vec3& v, double angle){
	double rotated_x = v.x * cos(angle) - v.y * sin(angle);
	double rotated_y = v.x * sin(angle) + v.y * cos(angle);
	return Vec3(rotated_x, rotated_y, v.z);
};

inline Vec3 Vec3_rotate_x(const Vec3& v, double angle){
	double rotated_y = v.y * cos(angle) - v.z * sin(angle);
	double rotated_z = v.y * sin(angle) + v.z * cos(angle);
	return Vec3(v.x, rotated_y, rotated_z);
};

inline Vec3 Vec3_rotate_y(const Vec3& v, double angle){
	double rotated_x = v.x * cos(angle) + v.z * sin(angle);
	double rotated_z = v.x * -sin(angle) + v.z * cos(angle);
	return Vec3(rotated_x, v.y, rotated_z);
}

inline Vec3 Vec3_from_vec4(const Vec4& v) { return Vec3(v.x, v.y, v.z); };
