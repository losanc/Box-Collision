#pragma once

#include <immintrin.h>
#include <iostream>


static const int absmaskint = 0x7FFFFFFF;
static const float absmask = *reinterpret_cast<const float*>(&absmaskint);
static const __m128 fourone = _mm_set_ps1(1.0f);
static const __m128 absfourmask = _mm_set_ps1(absmask);




struct vec4 {
	__m128 data;
	vec4() : data(_mm_setzero_ps()) {}
	vec4(float a, float b, float c, float d) : data(_mm_set_ps(d, c, b, a)) {}
	vec4(const __m128& d) : data(d) {}
	vec4(const vec4& rhs) : data(rhs.data) {}


	template<int i>
	inline float index() const {
		int res = _mm_extract_ps(data, i);
		return *reinterpret_cast<float*>(&res);
	}

	vec4 operator+(const vec4& rhs) const { return _mm_add_ps(data, rhs.data); }
	vec4 operator-(const vec4& rhs) const { return _mm_sub_ps(data, rhs.data); }
	inline friend vec4 operator*(const float a, const vec4& rhs) { return _mm_mul_ps(rhs.data, _mm_set1_ps(a)); }
	inline friend vec4 operator*(const vec4& lhs, const float a) { return _mm_mul_ps(lhs.data, _mm_set1_ps(a)); }
	inline friend vec4 operator/(const vec4& lhs, const float a) { return _mm_div_ps(lhs.data, _mm_set1_ps(a)); }

	inline bool HasOneNegative() const { return !(_mm_testz_ps(data, data)); }

	// from https://stackoverflow.com/a/35270026
	inline float dot(const vec4& rhs) const {
		__m128 v = _mm_mul_ps(data, rhs.data);
		__m128 shuf = _mm_movehdup_ps(v);        // broadcast elements 3,1 to 2,0
		__m128 sums = _mm_add_ps(v, shuf);
		shuf = _mm_movehl_ps(shuf, sums); // high half -> low half
		sums = _mm_add_ss(sums, shuf);
		return _mm_cvtss_f32(sums);
	}
	inline vec4 normalize() const {
		__m128 product = _mm_mul_ps(data, data);
		product = _mm_hadd_ps(product, product);
		product = _mm_hadd_ps(product, product);
		product = _mm_rsqrt_ps(product);
		return _mm_mul_ps(data, product);
	}
	inline void print() const {
		std::cout << index<0>() << "  " << index<1>() << "  " << index<2>() << "  " << index<3>() << "  " << std::endl;
	}
};

struct mat4 {
	vec4 row0;
	vec4 row1;
	vec4 row2;
	vec4 row3;
	mat4() : row0(1.0f, 0.f, 0.f, 0.f), row1(0.f, 1.f, 0.f, 0.f), row2(0.f, 0.f, 1.f, 0.f), row3(0.f, 0.f, 0.f, 1.f) {}
	mat4(vec4 row1, vec4 row2, vec4 row3, vec4 row4) : row0(row1), row1(row2), row2(row3), row3(row4) {}

	friend vec4 inline operator*(const mat4& lhs, const vec4& rhs) {
		return vec4(lhs.row0.dot(rhs), lhs.row1.dot(rhs), lhs.row2.dot(rhs), lhs.row3.dot(rhs));
	}
	inline mat4 operator*(const mat4& rhs) {
		mat4 res;
		res.row0 = row0.index<0>() * rhs.row0 + row0.index<1>() * rhs.row1 + row0.index<2>() * rhs.row2 + row0.index<3>() * rhs.row3;
		res.row1 = row1.index<0>() * rhs.row0 + row1.index<1>() * rhs.row1 + row1.index<2>() * rhs.row2 + row1.index<3>() * rhs.row3;
		res.row2 = row2.index<0>() * rhs.row0 + row2.index<1>() * rhs.row1 + row2.index<2>() * rhs.row2 + row2.index<3>() * rhs.row3;
		res.row3 = row3.index<0>() * rhs.row0 + row3.index<1>() * rhs.row1 + row3.index<2>() * rhs.row2 + row3.index<3>() * rhs.row3;
		return res;
	}
	inline mat4 Transpose() const {
		mat4 copy = *this;
		_MM_TRANSPOSE4_PS(copy.row0.data, copy.row1.data, copy.row2.data, copy.row3.data);
		return copy;
	}
	static inline mat4 Scale(float x, float y, float z) {
		vec4 row1(x, 0.f, 0.f, 0.f);
		vec4 row2(0.f, y, 0.f, 0.f);
		vec4 row3(0.f, 0.f, z, 0.f);
		vec4 row4(0.f, 0.f, 0.f, 1.f);
		return mat4(row1, row2, row3, row4);
	}

	static inline mat4 Translation(float x, float y, float z) {
		vec4 row1(1.f, 0.f, 0.f, x);
		vec4 row2(0.f, 1.f, 0.f, y);
		vec4 row3(0.f, 0.f, 1.f, z);
		vec4 row4(0.f, 0.f, 0.f, 1.f);
		return mat4(row1, row2, row3, row4);
	}
	static inline mat4 Rotation(vec4 quaternion) {
		vec4 normalized = quaternion.normalize();
		// xyz, w
		const float q0 = normalized.index<3>();
		const float q1 = normalized.index<0>();
		const float q2 = normalized.index<1>();
		const float q3 = normalized.index<2>();
		vec4 row1(2.0f * (q0 * q0 + q1 * q1) - 1.0f, 2.0f * (q1 * q2 - q0 * q3), 2.0f * (q1 * q3 + q0 * q2), 0.f);
		vec4 row2(2.0f * (q1 * q2 + q0 * q3), 2.0f * (q0 * q0 + q2 * q2) - 1.0f, 2.0f * (q2 * q3 - q0 * q1), 0.f);
		vec4 row3(2.0f * (q1 * q3 - q0 * q2), 2.0f * (q2 * q3 + q0 * q1), 2.0f * (q0 * q0 + q3 * q3) - 1.0f, 0.f);
		vec4 row4(0.f, 0.f, 0.f, 1.f);
		return mat4(row1, row2, row3, row4);
	}

	template<int i, int j>
	inline float index() const {
		if constexpr (i == 0) {
			return row0.index<j>();
		}
		if constexpr (i == 1) {
			return row1.index<j>();
		}
		if constexpr (i == 2) {
			return row2.index<j>();
		}
		if constexpr (i == 3) {
			return row3.index<j>();
		}
	}



	// from https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html
#define MakeShuffleMask(x,y,z,w)           (x | (y<<2) | (z<<4) | (w<<6))

// vec(0, 1, 2, 3) -> (vec[x], vec[y], vec[z], vec[w])
#define VecSwizzleMask(vec, mask)          _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), mask))
#define VecSwizzle(vec, x, y, z, w)        VecSwizzleMask(vec, MakeShuffleMask(x,y,z,w))
#define VecSwizzle1(vec, x)                VecSwizzleMask(vec, MakeShuffleMask(x,x,x,x))
// special swizzle
#define VecSwizzle_0022(vec)               _mm_moveldup_ps(vec)
#define VecSwizzle_1133(vec)               _mm_movehdup_ps(vec)

// return (vec1[x], vec1[y], vec2[z], vec2[w])
#define VecShuffle(vec1, vec2, x,y,z,w)    _mm_shuffle_ps(vec1, vec2, MakeShuffleMask(x,y,z,w))
// special shuffle
#define VecShuffle_0101(vec1, vec2)        _mm_movelh_ps(vec1, vec2)
#define VecShuffle_2323(vec1, vec2)        _mm_movehl_ps(vec2, vec1)
#define SMALL_NUMBER (1.e-8f)

	mat4 Inverse() const {
		//mat4 r;

		//// transpose 3x3, we know m03 = m13 = m23 = 0
		//__m128 t0 = VecShuffle_0101(row0.data, row1.data); // 00, 01, 10, 11
		//__m128 t1 = VecShuffle_2323(row0.data, row1.data); // 02, 03, 12, 13
		//r.row0 = VecShuffle(t0, row2.data, 0, 2, 0, 3); // 00, 10, 20, 23(=0)
		//r.row1 = VecShuffle(t0, row2.data, 1, 3, 1, 3); // 01, 11, 21, 23(=0)
		//r.row2 = VecShuffle(t1, row2.data, 0, 2, 2, 3); // 02, 12, 22, 23(=0)

		//// (SizeSqr(row1), SizeSqr(row2), SizeSqr(row3), 0)
		//__m128 sizeSqr;
		//sizeSqr = _mm_mul_ps(r.row0.data, r.row0.data);
		//sizeSqr = _mm_add_ps(sizeSqr, _mm_mul_ps(r.row1.data, r.row1.data));
		//sizeSqr = _mm_add_ps(sizeSqr, _mm_mul_ps(r.row2.data, r.row2.data));

		//// optional test to avoid divide by 0
		//__m128 one = _mm_set1_ps(1.f);
		//// for each component, if(sizeSqr < SMALL_NUMBER) sizeSqr = 1;
		//__m128 rSizeSqr = _mm_blendv_ps(
		//	_mm_div_ps(one, sizeSqr),
		//	one,
		//	_mm_cmplt_ps(sizeSqr, _mm_set1_ps(SMALL_NUMBER))
		//);

		//r.row0 = _mm_mul_ps(r.row0.data, rSizeSqr);
		//r.row1 = _mm_mul_ps(r.row1.data, rSizeSqr);
		//r.row2 = _mm_mul_ps(r.row2.data, rSizeSqr);

		//// last line
		//r.row3 = _mm_mul_ps(r.row0.data, VecSwizzle1(row3.data, 0));
		//r.row3 = _mm_add_ps(r.row3.data, _mm_mul_ps(r.row1.data, VecSwizzle1(row3.data, 1)));
		//r.row3 = _mm_add_ps(r.row3.data, _mm_mul_ps(r.row2.data, VecSwizzle1(row3.data, 2)));
		//r.row3 = _mm_sub_ps(_mm_setr_ps(0.f, 0.f, 0.f, 1.f), r.row3.data);

		//return r;

		const float A2323 = index<2, 2>() * index<3, 3>() - index<2, 3>() * index<3, 2>();
		const float A1323 = index<2, 1>() * index<3, 3>() - index<2, 3>() * index<3, 2>();
		const float A1223 = index<2, 1>() * index<3, 2>() - index<2, 2>() * index<3, 2>();
		const float A0323 = index<2, 0>() * index<3, 3>() - index<2, 3>() * index<3, 0>();
		const float A0223 = index<2, 0>() * index<3, 2>() - index<2, 2>() * index<3, 0>();
		const float A0123 = index<2, 0>() * index<3, 2>() - index<2, 1>() * index<3, 0>();
		const float A2313 = index<1, 2>() * index<3, 3>() - index<1, 3>() * index<3, 2>();
		const float A1313 = index<1, 1>() * index<3, 3>() - index<1, 3>() * index<3, 2>();
		const float A1213 = index<1, 1>() * index<3, 2>() - index<1, 2>() * index<3, 2>();
		const float A2312 = index<1, 2>() * index<2, 3>() - index<1, 3>() * index<2, 2>();
		const float A1312 = index<1, 1>() * index<2, 3>() - index<1, 3>() * index<2, 1>();
		const float A1212 = index<1, 1>() * index<2, 2>() - index<1, 2>() * index<2, 1>();
		const float A0313 = index<1, 0>() * index<3, 3>() - index<1, 3>() * index<3, 0>();
		const float A0213 = index<1, 0>() * index<3, 2>() - index<1, 2>() * index<3, 0>();
		const float A0312 = index<1, 0>() * index<2, 3>() - index<1, 3>() * index<2, 0>();
		const float A0212 = index<1, 0>() * index<2, 2>() - index<1, 2>() * index<2, 0>();
		const float A0113 = index<1, 0>() * index<3, 2>() - index<1, 1>() * index<3, 0>();
		const float A0112 = index<1, 0>() * index<2, 1>() - index<1, 1>() * index<2, 0>();

		float det = index<0, 0>() * (index<1, 1>() * A2323 - index<1, 2>() * A1323 + index<1, 3>() * A1223)
			- index<0, 1>() * (index<1, 0>() * A2323 - index<1, 2>() * A0323 + index<1, 3>() * A0223)
			+ index<0, 2>() * (index<1, 0>() * A1323 - index<1, 1>() * A0323 + index<1, 3>() * A0123)
			- index<0, 3>() * (index<1, 0>() * A1223 - index<1, 1>() * A0223 + index<1, 2>() * A0123);
		det = 1.0f / det;

		return mat4(
			vec4(det * (index<1, 1>() * A2323 - index<1, 2>() * A1323 + index<1, 3>() * A1223), det * -(index<0, 1>() * A2323 - index<0, 2>() * A1323 + index<0, 3>() * A1223), det * (index<0, 1>() * A2313 - index<0, 2>() * A1313 + index<0, 3>() * A1213), det * -(index<0, 1>() * A2312 - index<0, 2>() * A1312 + index<0, 3>() * A1212)),
			vec4(det * -(index<1, 0>() * A2323 - index<1, 2>() * A0323 + index<1, 3>() * A0223), det * (index<0, 0>() * A2323 - index<0, 2>() * A0323 + index<0, 3>() * A0223), det * -(index<0, 0>() * A2313 - index<0, 2>() * A0313 + index<0, 3>() * A0213), det * (index<0, 0>() * A2312 - index<0, 2>() * A0312 + index<0, 3>() * A0212)),
			vec4(det * (index<1, 0>() * A1323 - index<1, 1>() * A0323 + index<1, 3>() * A0123), det * -(index<0, 0>() * A1323 - index<0, 1>() * A0323 + index<0, 3>() * A0123), det * (index<0, 0>() * A1313 - index<0, 1>() * A0313 + index<0, 3>() * A0113), det * -(index<0, 0>() * A1312 - index<0, 1>() * A0312 + index<0, 3>() * A0112)),
			vec4(det * -(index<1, 0>() * A1223 - index<1, 1>() * A0223 + index<1, 2>() * A0123), det * (index<0, 0>() * A1223 - index<0, 1>() * A0223 + index<0, 2>() * A0123), det * -(index<0, 0>() * A1213 - index<0, 1>() * A0213 + index<0, 2>() * A0113), det * (index<0, 0>() * A1212 - index<0, 1>() * A0212 + index<0, 2>() * A0112))
		);

	}

	inline void print() const {
		row0.print();
		row1.print();
		row2.print();
		row3.print();
	}
};