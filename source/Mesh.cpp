#include "pch.h"
#include "Mesh.h"
#include "Effect.h"
#include "Texture.h"

namespace dae
{
	Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const MeshDataPaths& paths)
		: m_pEffect{ new Effect{ pDevice, paths.effect } }
	{
		///Create textures
		
		m_pDiffuseTexture = new Texture{ pDevice, paths.diffuse };
		m_pEffect->SetDiffuseMap(m_pDiffuseTexture);

		if (!paths.normal.empty())
		{
			m_pNormalTexture = new Texture{ pDevice, paths.normal };
			m_pEffect->SetNormalMap(m_pNormalTexture);
		}
		if (!paths.specular.empty())
		{
			m_pSpecularTexture = new Texture{ pDevice, paths.specular };
			m_pEffect->SetSpecularMap(m_pSpecularTexture);
		}
		if (!paths.gloss.empty())
		{
			m_pGlossinessTexture = new Texture{ pDevice, paths.gloss };
			m_pEffect->SetGlossinessMap(m_pGlossinessTexture);
		}

		//Get Technique from Effect
		m_pTechnique = m_pEffect->GetTechnique();

		//Create Vertex Layout
		static constexpr uint32_t numElements{ 4 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "NORMAL";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[1].AlignedByteOffset = 12;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[2].SemanticName = "TANGENT";
		vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[2].AlignedByteOffset = 24;
		vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[3].SemanticName = "TEXCOORD";
		vertexDesc[3].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[3].AlignedByteOffset = 36;
		vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		//Create Input Layout and quit if failed
		D3DX11_PASS_DESC passDesc{};
		m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

		HRESULT result{
				pDevice->CreateInputLayout(
				vertexDesc,
				numElements,
				passDesc.pIAInputSignature,
				passDesc.IAInputSignatureSize,
				&m_pInputLayout
			)
		};

		if (FAILED(result))
		{
			return;
		}

		//Create vertex buffer and quit if failed
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(vertices.size());
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = vertices.data();

		result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);

		if (FAILED(result))
		{
			return;
		}

		//Create index buffer and quit if failed
		m_NumIndices = static_cast<uint32_t>(indices.size());
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		initData.pSysMem = indices.data();

		result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);

		if (FAILED(result))
		{
			return;
		}
	}

	dae::Mesh::~Mesh()
	{
		if (m_pIndexBuffer)
		{
			m_pIndexBuffer->Release();
		}

		if (m_pInputLayout)
		{
			m_pInputLayout->Release();
		}

		if (m_pVertexBuffer)
		{
			m_pVertexBuffer->Release();
		}

		delete m_pEffect;
		delete m_pDiffuseTexture;
		delete m_pNormalTexture;
		delete m_pSpecularTexture;
		delete m_pGlossinessTexture;
	}
	void Mesh::Render(ID3D11DeviceContext* pDeviceContext) const
	{
		//1. Set Primitive Topology
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//2. Set Input Layout
		pDeviceContext->IASetInputLayout(m_pInputLayout);

		//3. Set Vertex Buffer
		constexpr UINT stride{ sizeof(Vertex) };
		constexpr UINT offset{ 0 };
		pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

		//4. Set Index Buffer
		pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		//5. Draw
		D3DX11_TECHNIQUE_DESC techDesc{};
		m_pTechnique->GetDesc(&techDesc);
		for (UINT p{ 0 }; p < techDesc.Passes; ++p)
		{
			m_pTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext);
			pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
		}
	}

	void Mesh::SetMatrices(const Matrix& viewProj, const Matrix& invView) const
	{
		m_pEffect->SetMatrixWorld(m_RotationMatrix);
		m_pEffect->SetMatrixViewProj(m_RotationMatrix * viewProj);
		m_pEffect->SetMatrixViewInv(invView);
	}

	ID3DX11EffectSamplerVariable* Mesh::GetSampleVar() const
	{
		return m_pEffect->GetEffect()->GetVariableByName("gSampler")->AsSampler();
	}

	ID3DX11EffectRasterizerVariable* Mesh::GetRasterizer() const
	{
		return m_pEffect->GetEffect()->GetVariableByName("gRasterizerState")->AsRasterizer();
	}
}
