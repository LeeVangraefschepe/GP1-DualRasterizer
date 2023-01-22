#include "pch.h"
#include "HardwareTexture.h"

HardwareTexture::HardwareTexture(ID3D11Device* pDevice, const std::string& path)
{
	//Load texture image
	SDL_Surface* pSurface = IMG_Load(path.c_str());

	//Set texture settings for directX
	constexpr DXGI_FORMAT format{ DXGI_FORMAT_R8G8B8A8_UNORM };
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = pSurface->w;
	desc.Height = pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

	//Create texture on GPU
	HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pTexture2D);
	if (FAILED(hr))
	{
		return;
	}

	//Set the shader resource view description
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	//Create the shader resource view on GPU
	hr = pDevice->CreateShaderResourceView(m_pTexture2D, &SRVDesc, &m_pSRV);
	if (FAILED(hr))
	{
		return;
	}

	//Release sdl surface
	SDL_FreeSurface(pSurface);
}

HardwareTexture::~HardwareTexture()
{
	m_pTexture2D->Release();
	m_pSRV->Release();
}

ID3D11Texture2D* HardwareTexture::GetTexture2D() const
{
	return m_pTexture2D;
}

ID3D11ShaderResourceView* HardwareTexture::GetSRV() const
{
	return m_pSRV;
}