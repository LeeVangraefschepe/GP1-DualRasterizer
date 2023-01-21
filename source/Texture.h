#pragma once
class Texture final
{
public:
	Texture(ID3D11Device* pDevice, const std::string& path);
	~Texture();

	ID3D11Texture2D* GetTexture2D() const;
	ID3D11ShaderResourceView* GetSRV() const;

	Texture(Texture&&) noexcept = delete;
	Texture& operator=(const Texture&) = delete;
	Texture& operator=(Texture&&) noexcept = delete;
private:
	ID3D11Texture2D* m_pTexture2D{};
	ID3D11ShaderResourceView* m_pSRV{};
};