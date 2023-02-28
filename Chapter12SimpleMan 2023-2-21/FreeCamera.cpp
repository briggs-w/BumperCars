#include "FreeCamera.h"
//#include "MathExtras.h"

#include <iostream>
#include "Math.h"

namespace AssimpPlus
{

	FreeCamera::FreeCamera(const Vector3& position, const Vector3& lookAt, const Vector3& up)
		: position_(position)
		, forward_(Vector3(Vector3().Normalize(Vector3(lookAt) - Vector3(position))))
		, up_(Vector3::Normalize(up))
		, right_(Vector3::Normalize(Vector3::Cross(up, lookAt - position)))
		, dViewMatrix_({ Matrix4::Identity, true })
	{}

	Matrix4 FreeCamera::GetViewMatrix()
	{
		if (dViewMatrix_.isDirty)
		{
			dViewMatrix_.viewMatrix = Matrix4::LookAtLH(position_, position_ + forward_, up_);
			dViewMatrix_.isDirty = false;
		}

		return dViewMatrix_.viewMatrix;
	}

	Vector3 FreeCamera::GetPosition() const
	{
		return position_;
	}

	void FreeCamera::MoveForward(float distance)
	{
		// Here is one of the reasons I love C++ operator overloading.
		// This here makes really intuitive sense to a beginner looking at it
		// At least, it did for me. Move the position in the direction of forward by some distance "distance"
		position_ += forward_ * distance;

		dViewMatrix_.isDirty = true;
	}

	void FreeCamera::MoveRight(float distance)
	{
		position_ += right_ * distance;

		dViewMatrix_.isDirty = true;
	}

	void FreeCamera::MoveUp(float distance)
	{
		position_ += up_ * distance;

		dViewMatrix_.isDirty = true;
	}

	void FreeCamera::RotateRight(float angle)
	{
		// The up axis doesn't change, so just rotate about that
		::Quaternion rotation(up_, angle);
		//forward_ = Vector3::Normalize(forward_ * rotation);
		forward_ = Vector3::Normalize(forward_ * Vector3(rotation.x, rotation.y, rotation.z));
		

		right_ = Vector3::Normalize(Vector3::Cross(up_, forward_));

		dViewMatrix_.isDirty = true;
	}

	void FreeCamera::RotateUp(float angle)
	{
		// The right axis should be parallel to the ground, so it can safely be used.
		::Quaternion rotation(right_, angle);
		//Vector3 newForward = Vector3::Normalize(forward_ * rotation);
		Vector3 newForward = Vector3::Normalize(forward_ * Vector3(rotation.x, rotation.y, rotation.z));

		if (Vector3::Cross(newForward, up_).Length() < 0.01f)
		{
			// The two vectors are too close for comfort. Don't apply this operation
			return;
		}
		forward_ = newForward;
		right_ = Vector3::Normalize(Vector3::Cross(up_, forward_));

		dViewMatrix_.isDirty = true;
	}

};