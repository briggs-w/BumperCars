#include "Transform.h"

namespace AssimpPlus
{
	Matrix4 getTransformMatrix(const Vector3& Position, const ::Quaternion& Rotation, float scale)
	{
		const Vector3 Scale(scale, scale, scale);
		Matrix4 m;

		m.mat[0][0] = Scale.x * (1.f - 2.f * Rotation.y * Rotation.y - 2.f * Rotation.z * Rotation.z);
		m.mat[0][1] = Scale.y * (2.f * Rotation.x * Rotation.y - 2.f * Rotation.z * Rotation.w);
		m.mat[0][2] = Scale.z * (2.f * Rotation.x * Rotation.z + 2.f * Rotation.y * Rotation.w);
		m.mat[0][3] = Position.x;

		m.mat[1][0] = Scale.x * (2.f * Rotation.x * Rotation.y + 2.f * Rotation.z * Rotation.w);
		m.mat[1][1] = Scale.y * (1.f - 2.f * Rotation.x * Rotation.x - 2.f * Rotation.z * Rotation.z);
		m.mat[1][2] = Scale.z * (2.f * Rotation.y * Rotation.z - 2.f * Rotation.x * Rotation.w);
		m.mat[1][3] = Position.y;

		m.mat[2][0] = Scale.x * (2.f * Rotation.x * Rotation.z - 2.f * Rotation.y * Rotation.w);
		m.mat[2][1] = Scale.y * (2.f * Rotation.y * Rotation.z + 2.f * Rotation.x * Rotation.w);
		m.mat[2][2] = Scale.z * (1.f - 2.f * Rotation.x * Rotation.x - 2.f * Rotation.y * Rotation.y);
		m.mat[2][3] = Position.z;

		m.mat[3][0] = 0.f;
		m.mat[3][1] = 0.f;
		m.mat[3][2] = 0.f;
		m.mat[3][3] = 1.f;

		return m;
	} 
};