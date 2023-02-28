#pragma once

//taken from indego code
//this might be able to be rolled into math.h

// Represents a 3D point transform - translation, rotation, scaling.
// It's much easier to work with this kind of class on the logical side, and
//  convert to a matrix before sending to the GPU.
// Realistically matrices could be exclusively used, but it's much easier to debug
//  and reason about code in this format.
// Is it faster or slower? No clue. I actually haven't profiled it.

#include "Math.h"

namespace AssimpPlus
{
	Matrix4 getTransformMatrix(const Vector3& Position, const ::Quaternion& Rotation, float Scale);
};