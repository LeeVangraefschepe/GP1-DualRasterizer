#pragma once
#include "pch.h"

class Effect;
class HardwareTexture;

namespace dae
{
	struct Vertex
	{
		Vector3 Position;
		Vector3 Normal;
		Vector3 Tangent;
		Vector2 UV;
	};

	struct MeshDataPaths
	{
		std::wstring effect;
		std::string diffuse;
		std::string normal;
		std::string specular;
		std::string gloss;
		void Clear()
		{
			effect.clear();
			diffuse.clear();
			normal.clear();
			specular.clear();
			gloss.clear();
		}
	};

	class HardwareMesh final
	{
	public:

		explicit HardwareMesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const MeshDataPaths& paths, Matrix* pWorldMatrix);
		~HardwareMesh();

		HardwareMesh(const HardwareMesh&) = delete;
		HardwareMesh(HardwareMesh&&) noexcept = delete;
		HardwareMesh& operator=(const HardwareMesh&) = delete;
		HardwareMesh& operator=(HardwareMesh&&) noexcept = delete;

		void Render(ID3D11DeviceContext* pDeviceContext) const;

		void SetMatrices(const Matrix& viewProj, const Matrix& invView) const;

		void RotateY(float rotation) const
		{
			*m_pWorldMatrix = Matrix::CreateRotationY(rotation) * *m_pWorldMatrix;
		}

		ID3DX11EffectSamplerVariable* GetSampleVar() const;
		ID3DX11EffectRasterizerVariable* GetRasterizer() const;
	private:

		Effect* m_pEffect{};
		HardwareTexture* m_pDiffuseTexture{};
		HardwareTexture* m_pNormalTexture{};
		HardwareTexture* m_pSpecularTexture{};
		HardwareTexture* m_pGlossinessTexture{};
		ID3DX11EffectTechnique* m_pTechnique{};

		ID3D11Buffer* m_pVertexBuffer{};
		ID3D11InputLayout* m_pInputLayout{};
		ID3D11Buffer* m_pIndexBuffer{};

		uint32_t m_NumIndices{};

		Matrix* m_pWorldMatrix;
	};
}