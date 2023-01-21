#pragma once
#include "Utils.h"
#include "Camera.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	enum SampleState
	{
		Point,
		Linear,
		Anisotropic
	};

	enum CullMode
	{
		Back,
		Front,
		None
	};

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow, Camera* pCamera);
		~Renderer();

		void ToggleRotation() { m_Rotate = !m_Rotate; }
		void CycleCullModes();
		void ToggleClearCollor() { m_ClearColor = !m_ClearColor; }

		void ToggleFireMesh() { m_ShowFireMesh = !m_ShowFireMesh; }
		void CycleSampleStates();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

	private:
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		//DIRECTX
		HRESULT InitializeDirectX();
		ID3D11Device* m_pDevice{ nullptr };
		ID3D11DeviceContext* m_pDeviceContext{ nullptr };
		IDXGISwapChain* m_pSwapChain{ nullptr };
		ID3D11Texture2D* m_pDepthStencilBuffer{ nullptr };
		ID3D11DepthStencilView* m_pDepthStencilView{ nullptr };
		ID3D11Resource* m_pRenderTargetBuffer{ nullptr };
		ID3D11RenderTargetView* m_pRenderTargetView{ nullptr };

		ID3DX11EffectSamplerVariable* m_pEffectSamplerVariable{};
		ID3D11SamplerState* m_pSamplerState{};
		ID3D11RasterizerState* m_pRasterizerState{};
		D3D11_SAMPLER_DESC m_SamplerDesc{};

		//GPU SLOT
		const UINT m_GpuIndex{ 0 }; //0 default

		//Render settings
		bool m_Rotate{ true };
		CullMode m_CullMode{ None };
		bool m_ClearColor{ true };

		//DirectX only
		bool m_ShowFireMesh{ true };
		SampleState m_SampleState{ Point };

		std::vector<Mesh*> m_pMeshes{};
		Camera* m_pCamera{};

		void CreateMesh();
	};
}
