

//from indego code

#include "Quaternion.h"
#include <algorithm>
//#include <math.h> //from madhav, breaks things but will eventually replace quaternion.h with it
namespace AssimpPlus
{
	const Quaternion Quaternion::Identity = Quaternion(0.f, 0.f, 1.f, 0.f);

	// Unit quaternion: Arbitrary axis, angle of 0
	Quaternion::Quaternion() : x(0.f), y(0.f), z(0.f), w(1.f)
	{}

	/*Quaternion::Quaternion(Vec3 axis, float angle)
		: w(cosf(angle / 2.f))
		, x(sinf(angle / 2.f)* axis.x)
		, y(sinf(angle / 2.f)* axis.y)
		, z(sinf(angle / 2.f)* axis.z)
	{
		Normalize();
	}*/

	Quaternion::Quaternion(float X, float Y, float Z, float W)
		: w(W)
		, x(X)
		, y(Y)
		, z(Z)
	{
		Normalize();
	}

	Quaternion Quaternion::Inverse() const
	{
		return Quaternion(x, y, z, - w);
	}


	//this is the same thing as Concatenate in math.h i believe
	Quaternion Quaternion::operator*(const Quaternion& o) const
	{
		// Multiplying two quaternions together has the effect of performing the
		//  first rotation, and then performing the second.
		return Quaternion(
			o.w * z - o.x * y + o.y * x + o.z * w,
			o.w * w - o.x * x - o.y * y - o.z * z,
			o.w * x + o.x * w - o.y * z + o.z * y,
			o.w * y + o.x * z + o.y * w - o.z * x);
	}

	Quaternion& Quaternion::operator*=(const Quaternion& o)
	{
		return *this = *this * o;
	}

	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
	Quaternion Quaternion::FromMatrix(const Matrix& m)
	{
		float sx = Vec3(m._11, m._21, m._31).Length();
		float sy = Vec3(m._12, m._22, m._32).Length();
		float sz = Vec3(m._13, m._23, m._33).Length();

		float w = sqrtf(std::max(0.f, 1.f + m._11 / sx + m._22 / sy + m._33 / sz)) / 2.f;
		float x = copysignf(sqrtf(std::max(0.f, 1.f + m._11 / sx - m._22 / sy - m._33 / sz)) / 2.f, m._32 - m._23);
		float y = copysignf(sqrtf(std::max(0.f, 1.f - m._11 / sx + m._22 / sy - m._33 / sz)) / 2.f, m._13 - m._31);
		float z = copysignf(sqrtf(std::max(0.f, 1.f - m._11 / sx - m._22 / sy + m._33 / sz)) / 2.f, m._21 - m._12);

		return Quaternion(x, y, z, w);
	}


	//
	void Quaternion::Normalize()
	{
		float mag = sqrt(x * x + w * w + y * y + z * z);
		x /= mag;
		y /= mag;
		z /= mag;
		w /= mag;
	}
}
