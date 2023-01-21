#include "pch.h"
#include "Effect.h"

Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
	: m_pEffect{ LoadEffect(pDevice, assetFile) }
{
	if (m_pEffect == nullptr)
	{
		std::wcout << L"Failed to load effect from file.\n";
		return;
	}

	///Get pointers from shader en check if valid
	m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
	if (!m_pTechnique->IsValid())
	{
		std::wcout << L"Technique not valid!\n";
	}

	//Matrices
	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVariable->IsValid())
	{
		std::wcout << L"m_pMatWorldViewProjVariable not valid!\n";
	}
	m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
	if (!m_pMatWorldVariable->IsValid())
	{
		std::wcout << L"m_pMatWorldVariable not valid!\n";
	}
	m_pMatViewInvVariable = m_pEffect->GetVariableByName("gViewInverseMatrix")->AsMatrix();
	if (!m_pMatViewInvVariable->IsValid())
	{
		std::wcout << L"m_pMatViewInvVariable not valid!\n";
	}

	//Textures
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"m_pDiffuseMapVariable not valid!\n";
	}
	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVariable->IsValid())
	{
		std::wcout << L"m_pNormalMapVariable not valid!\n";
	}
	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVariable->IsValid())
	{
		std::wcout << L"m_pSpecularMapVariable not valid!\n";
	}
	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!m_pGlossinessMapVariable->IsValid())
	{
		std::wcout << L"m_pGlossinessMapVariable not valid!\n";
	}
}

Effect::~Effect()
{
	if (m_pTechnique)
	{
		m_pTechnique->Release();
	}

	//Matrices
	if (m_pMatViewInvVariable)
	{
		m_pMatViewInvVariable->Release();
	}
	if (m_pMatWorldVariable)
	{
		m_pMatWorldVariable->Release();
	}
	if (m_pMatWorldViewProjVariable)
	{
		m_pMatWorldViewProjVariable->Release();
	}

	//Textures
	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->Release();
	}

	//Whole effect
	if (m_pEffect)
	{
		m_pEffect->Release();
	}
}

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result;
	ID3D10Blob* pErrorBlob{ nullptr };
	ID3DX11Effect* pEffect{ nullptr };

	DWORD shaderFlags{ 0 };
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(
		assetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob
	);

	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
		{
			const char* pErrors{ static_cast<char*>(pErrorBlob->GetBufferPointer()) };

			std::wstringstream ss;
			for (unsigned int i{ 0 }; i < pErrorBlob->GetBufferSize(); ++i)
			{
				ss << pErrors[i];
			}

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << "\n";
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
			std::wcout << ss.str() << "\n";
			return nullptr;
		}
	}

	return pEffect;
}

void Effect::SetDiffuseMap(const HardwareTexture* pDiffuseTexture) const
{
	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetSRV());
	}
}
void Effect::SetNormalMap(const HardwareTexture* pDiffuseTexture) const
{
	if (m_pNormalMapVariable)
	{
		m_pNormalMapVariable->SetResource(pDiffuseTexture->GetSRV());
	}
}
void Effect::SetSpecularMap(const HardwareTexture* pDiffuseTexture) const
{
	if (m_pSpecularMapVariable)
	{
		m_pSpecularMapVariable->SetResource(pDiffuseTexture->GetSRV());
	}
}
void Effect::SetGlossinessMap(const HardwareTexture* pDiffuseTexture) const
{
	if (m_pGlossinessMapVariable)
	{
		m_pGlossinessMapVariable->SetResource(pDiffuseTexture->GetSRV());
	}
}
void Effect::SetMatrixViewProj(const dae::Matrix& matrix) const
{
	m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&matrix));
}
void Effect::SetMatrixWorld(const dae::Matrix& matrix) const
{
	m_pMatWorldVariable->SetMatrix(reinterpret_cast<const float*>(&matrix));
}
void Effect::SetMatrixViewInv(const dae::Matrix& matrix) const
{
	m_pMatViewInvVariable->SetMatrix(reinterpret_cast<const float*>(&matrix));
}