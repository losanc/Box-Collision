#pragma once

#include <immintrin.h>
#include "vec.h"


struct Cube {

	vec4 m_scale{ 1.f, 1.f, 1.f, 1.f };
	vec4 m_translation{ 0.f,0.f,0.f,1.f };
	vec4 m_rotation{ 0.f,0.f,0.f,1.f };

	inline mat4 GetTransform() const {
		mat4 scale = mat4::Scale(m_scale.index<0>(), m_scale.index<1>(), m_scale.index<2>());
		mat4 rotation = mat4::Rotation(m_rotation);
		mat4 translation = mat4::Translation(m_translation.index<0>(), m_translation.index<1>(), m_translation.index<2>());
		return translation * rotation * scale;
	}
	inline mat4 GetTransformInv() const {
		return GetTransform().Inverse();
	}

	inline bool Collision(const Cube& another) const {
		return CollisionSingleSide(another) && another.CollisionSingleSide(*this);
	}

	inline bool CollisionSingleSide(const Cube& another) const {

		const mat4 transform = GetTransformInv() * another.GetTransform();
		const mat4 transpose = transform.Transpose();
		const vec4 abscol0 = _mm_and_ps(absfourmask, transpose.row0.data);
		const vec4 abscol1 = _mm_and_ps(absfourmask, transpose.row1.data);
		const vec4 abscol2 = _mm_and_ps(absfourmask, transpose.row2.data);
		const vec4 abscol3 = _mm_and_ps(absfourmask, transpose.row3.data);

		// vertex face test
		{
			const vec4 range = abscol0 + abscol1 + abscol2 + fourone;
			const vec4 test = range - abscol3;
			if (test.HasOneNegative())
				return false;
		}
		// edge edge test
		{
			const vec4 abscol0shufle = _mm_permute_ps(abscol0.data, 0b11010010);
			const vec4 abscol1shufle = _mm_permute_ps(abscol1.data, 0b11010010);
			const vec4 abscol2shufle = _mm_permute_ps(abscol2.data, 0b11010010);

			const vec4 col0shufle = _mm_permute_ps(transpose.row0.data, 0b11010010);
			const vec4 col1shufle = _mm_permute_ps(transpose.row1.data, 0b11010010);
			const vec4 col2shufle = _mm_permute_ps(transpose.row2.data, 0b11010010);
			const vec4 col3shufle = _mm_permute_ps(transpose.row3.data, 0b11010010);

			{
				const vec4 threshold = abscol0 + abscol0shufle;
				const vec4 range11 = _mm_add_ps(_mm_mul_ps(abscol0.data, abscol1shufle.data), _mm_mul_ps(abscol0shufle.data, abscol1.data));
				const vec4 range12 = _mm_add_ps(_mm_mul_ps(abscol0.data, abscol2shufle.data), _mm_mul_ps(abscol0shufle.data, abscol2.data));
				const vec4 range1 = range11 + range12;
				const vec4 center1 = _mm_sub_ps(_mm_mul_ps(transpose.row0.data, col3shufle.data), _mm_mul_ps(col0shufle.data, transpose.row3.data));
				const vec4 abscenter = _mm_and_ps(absfourmask, center1.data);
				const vec4 test = threshold + range1 - abscenter;
				if (test.HasOneNegative())
					return false;
			}
			{
				const vec4 threshold = abscol1 + abscol1shufle;
				const vec4 range11 = _mm_add_ps(_mm_mul_ps(abscol1.data, abscol0shufle.data), _mm_mul_ps(abscol1shufle.data, abscol0.data));
				const vec4 range12 = _mm_add_ps(_mm_mul_ps(abscol1.data, abscol2shufle.data), _mm_mul_ps(abscol1shufle.data, abscol2.data));
				const vec4 range1 = range11 + range12;
				const vec4 center1 = _mm_sub_ps(_mm_mul_ps(transpose.row1.data, col3shufle.data), _mm_mul_ps(col1shufle.data, transpose.row3.data));

				const vec4 abscenter = _mm_and_ps(absfourmask, center1.data);
				const vec4 test = threshold + range1 - abscenter;
				if (test.HasOneNegative())
					return false;
			}
			{
				const vec4 threshold = abscol2 + abscol2shufle;
				const vec4 range11 = _mm_add_ps(_mm_mul_ps(abscol2.data, abscol0shufle.data), _mm_mul_ps(abscol2shufle.data, abscol0.data));
				const vec4 range12 = _mm_add_ps(_mm_mul_ps(abscol2.data, abscol1shufle.data), _mm_mul_ps(abscol2shufle.data, abscol1.data));
				const vec4 range1 = range11 + range12;
				const vec4 center1 = _mm_sub_ps(_mm_mul_ps(transpose.row2.data, col3shufle.data), _mm_mul_ps(col2shufle.data, transpose.row3.data));

				const vec4 abscenter = _mm_and_ps(absfourmask, center1.data);
				const vec4 test = threshold + range1 - abscenter;
				if (test.HasOneNegative())
					return false;
			}
		}
		return true;
	}
};