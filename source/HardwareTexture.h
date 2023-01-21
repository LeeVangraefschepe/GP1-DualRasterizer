#pragma once
class HardwareTexture final
{
public:
	HardwareTexture(ID3D11Device* pDevice, const std::string& path);
	~HardwareTexture();

	ID3D11Texture2D* GetTexture2D() const;
	ID3D11ShaderResourceView* GetSRV() const;

	HardwareTexture(HardwareTexture&&) noexcept = delete;
	HardwareTexture& operator=(const HardwareTexture&) = delete;
	HardwareTexture& operator=(HardwareTexture&&) noexcept = delete;
private:
	ID3D11Texture2D* m_pTexture2D{};
	ID3D11ShaderResourceView* m_pSRV{};
};