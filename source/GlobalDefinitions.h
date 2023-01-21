#pragma once
#include "HardwareMesh.h"
struct SoftwareMesh;

using namespace dae;

enum CullMode
{
	Back,
	Front,
	None
};

struct GlobalMesh final
{
	~GlobalMesh()
	{
		delete pWorldMatrix;
	}
	HardwareMesh* pHMesh{nullptr};
	SoftwareMesh* pSMesh{nullptr};
	Matrix* pWorldMatrix = new Matrix{ Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, Vector3::Zero };
};
