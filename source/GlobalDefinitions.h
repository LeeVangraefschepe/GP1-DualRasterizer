#pragma once
namespace dae
{
	class HardwareMesh;
	struct SoftwareMesh;

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

		HardwareMesh* pHMesh{ nullptr };
		SoftwareMesh* pSMesh{ nullptr };
		Matrix* pWorldMatrix = new dae::Matrix{ dae::Vector3::UnitX, dae::Vector3::UnitY, dae::Vector3::UnitZ, dae::Vector3::Zero };
	};
}