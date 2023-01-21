#include "pch.h"
#include "Renderer.h"
#include <future>

#define DEBUG

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow, Camera* pCamera) :
		m_pWindow(pWindow),
		m_pCamera(pCamera)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		CreateMesh();

		//Initialize sampler
		m_pEffectSamplerVariable = m_pMeshes[0]->GetSampleVar();
		if (!m_pEffectSamplerVariable->IsValid())
		{
			std::wcout << L"m_pEffectSamplerVariable not valid!\n";
		}

		//Create sampler
		m_SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		m_SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		m_SamplerDesc.MipLODBias = 0;
		m_SamplerDesc.MinLOD = 0;

		m_SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		m_SamplerDesc.MaxAnisotropy = 16;
	}

	Renderer::~Renderer()
	{
		if (m_pRenderTargetView)
		{
			m_pRenderTargetView->Release();
		}
		if (m_pRenderTargetBuffer)
		{
			m_pRenderTargetBuffer->Release();
		}
		if (m_pDepthStencilView)
		{
			m_pDepthStencilView->Release();
		}
		if (m_pDepthStencilBuffer)
		{
			m_pDepthStencilBuffer->Release();
		}
		if (m_pSwapChain)
		{
			m_pSwapChain->Release();
		}
		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}
		if (m_pDevice)
		{
			m_pDevice->Release();
		}
		if (m_pEffectSamplerVariable)
		{
			m_pEffectSamplerVariable->Release();
		}
		if (m_pSamplerState)
		{
			m_pSamplerState->Release();
		}
		if (m_pRasterizerState)
		{
			m_pRasterizerState->Release();
		}

		for (const auto pMesh : m_pMeshes)
		{
			delete pMesh;
		}
	}

	void Renderer::Update(const Timer* pTimer)
	{
		constexpr float rotateSpeed{ 45.0f };

		for (const auto pMesh : m_pMeshes)
		{
			if (m_Rotate)
			{
				pMesh->RotateY(rotateSpeed * TO_RADIANS * pTimer->GetElapsed());
			}

			pMesh->SetMatrices(m_pCamera->viewMatrix * m_pCamera->projectionMatrix, m_pCamera->invViewMatrix);
		}
	}


	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;

		//Clear window for next frame
		ColorRGB clearColor{ 0.39f, 0.59f, 0.93f };
		if (m_ClearColor)
		{
			clearColor = {0.1f,0.1f,0.1f};
		}
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		//Set pipeline + invoke drawcalls (= render)
		const int size = static_cast<int>(m_pMeshes.size());
		for (int i{}; i < size; ++i)
		{
			if (m_ShowFireMesh == false && i > 0)
			{
				break;
			}
			m_pMeshes[i]->Render(m_pDeviceContext);
		}
		

		//Present to screen
		m_pSwapChain->Present(0, 0);
	}

	HRESULT Renderer::InitializeDirectX()
	{
		///1. Create Device & DeviceContext
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		IDXGIFactory1* pFactory = nullptr;
		HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory);

		HRESULT result{};

		if (SUCCEEDED(hr))
		{
			IDXGIAdapter1* pAdapter = nullptr;

			hr = pFactory->EnumAdapters1(m_GpuIndex, &pAdapter);
			if (SUCCEEDED(hr))
			{
				// Create device and context on the second GPU
				result = D3D11CreateDevice(pAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);

				if (FAILED(result))
				{
					return result;
				}

				pAdapter->Release();
			}

			pFactory->Release();
		}

		

		IDXGIFactory1* pDxgiFactory{ nullptr };
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result))
		{
			return result;
		}

		///2. Create swapchain

		//Set properties of swapchain
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get the handle (HWND) from the SDL Backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Create swapchain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result))
		{
			return result;
		}

		///3. Create DepthStencil (DS) & DepthStencilView (DSV)

		//Set properties of depthstencil
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		//View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
		{
			return result;
		}

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
		{
			return result;
		}

		///4. Create RenderTarget (RT) and RenderTargetView (RTV)
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
		{
			return result;
		}

		//View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result))
		{
			return result;
		}

		///5. Bind RTV and DSV to Output Merger Stage
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		///6. Set viewport
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<FLOAT>(m_Width);
		viewport.Height = static_cast<FLOAT>(m_Height);
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		return S_OK;
	}

	void Renderer::CycleSampleStates()
	{
		m_SampleState = static_cast<SampleState>((static_cast<int>(m_SampleState) + 1) % 3);

		switch (m_SampleState)
		{
		case dae::Point:
			std::cout << "POINT\n";
			m_SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		case dae::Linear:
			std::cout << "LINEAR\n";
			m_SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case dae::Anisotropic:
			std::cout << "ANISOTROPIC\n";
			m_SamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			break;
		}

		if (m_pSamplerState) m_pSamplerState->Release();

		const HRESULT result{ m_pDevice->CreateSamplerState(&m_SamplerDesc, &m_pSamplerState) };

		if (FAILED(result)) return;

		m_pEffectSamplerVariable->SetSampler(0, m_pSamplerState);
	}
	void Renderer::CycleCullModes()
	{
		m_CullMode = static_cast<CullMode>((static_cast<int>(m_CullMode) + 1) % 3);

		D3D11_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.AntialiasedLineEnable = false;

		switch (m_CullMode)
		{
		case Back:
			rasterizerDesc.CullMode = D3D11_CULL_BACK;
			std::cout << "Set to back cull mode\n";
			break;
		case Front:
			rasterizerDesc.CullMode = D3D11_CULL_FRONT;
			std::cout << "Set to front cull mode\n";
			break;
		case None:
			rasterizerDesc.CullMode = D3D11_CULL_NONE;
			std::cout << "Set to none cull mode\n";
			break;
		}

		if (m_pRasterizerState)
		{
			m_pRasterizerState->Release();
		}

		const HRESULT hr{ m_pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerState) };
		if (FAILED(hr)) std::wcout << L"m_pRasterizerState failed to create\n";

		//for (const auto pMesh : m_pMeshes)
		//{
		//	pMesh->GetRasterizer()->SetRasterizerState(0, m_pRasterizerState);
		//}
		m_pMeshes[0]->GetRasterizer()->SetRasterizerState(0, m_pRasterizerState);
	}

	void Renderer::CreateMesh()
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};

		//Load main mesh
		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);

		MeshDataPaths paths;
		paths.effect = L"Resources/PosCol3D.fx";
		paths.diffuse = "Resources/vehicle_diffuse.png";
		paths.normal = "Resources/vehicle_normal.png";
		paths.specular = "Resources/vehicle_specular.png";
		paths.gloss = "Resources/vehicle_gloss.png";

		m_pMeshes.push_back(new Mesh{ m_pDevice, vertices, indices, paths});
		vertices.clear();
		indices.clear();
		paths.Clear();

		//Load fire mesh
		paths.effect = L"Resources/PosTrans3D.fx";
		paths.diffuse = "Resources/fireFX_diffuse.png";
		Utils::ParseOBJ("Resources/fireFX.obj", vertices, indices);
		m_pMeshes.push_back(new Mesh{ m_pDevice, vertices, indices, paths });
	}
}
