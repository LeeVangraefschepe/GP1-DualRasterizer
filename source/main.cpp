#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "HardwareRenderer.h"
#include "SoftwareRenderer.h"
#include "Camera.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

void PrintKeyInfo()
{
	SetConsoleTitle("DualRasterizer - Lee Vangraefschepe 2DAEGD15N");
	std::cout << "\x1B[2J\x1B[H"; //Clear console
	std::cout << "\033[33m"; //Set yellow
	std::cout << "[Key Bindings - SHARED]\n";
	std::cout << "  [F1]  Toggle Rasterizer Mode (HARDWARE/SOFTWARE)\n";
	std::cout << "  [F2]  Toggle Vehicle Rotation (ON/OFF)\n";
	std::cout << "  [F9]  Cycle CullMode (BACK/FRONT/NONE)\n";
	std::cout << "  [F10] Toggle Uniform ClearColor (ON/OFF)\n";
	std::cout << "  [F11] Toggle Print FPS (ON/OFF)\n";
	std::cout << "\n\033[32m"; //Set green
	std::cout << "[Key Bindings - HARDWARE]\n";
	std::cout << "  [F3]  Toggle FireFX (ON/OFF)\n";
	std::cout << "  [F4]  Cycle Sampler State (POINT/LINEAR/ANISOTROPIC)\n";
	std::cout << "\n\033[35m"; //Set purple
	std::cout << "[Key Bindings - SOFTWARE]\n";
	std::cout << "  [F5]  Cycle Shading Mode (COMBINED/OBSERVED_AREA/DIFFUSE/SPECULAR)\n";
	std::cout << "  [F6]  Toggle NormalMap (ON/OFF)\n";
	std::cout << "  [F7]  Toggle DepthBuffer Visualization (ON/OFF)\n";
	std::cout << "  [F8]  Toggle BoundingBox Visualization (ON/OFF)\n";
	std::cout << "\n\033[0m"; //Set default
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	constexpr uint32_t width = 640;
	constexpr uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"DualRasterizer - Lee Vangraefschepe 2DAEGD15N",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Create settings
	const auto pCullmode = new CullMode{ Back };
	bool isHardware{ true };
	bool showFps{};

	//Data between the two renders
	std::vector<GlobalMesh*> pGlobalMeshes{};
	pGlobalMeshes.push_back(new GlobalMesh{});
	pGlobalMeshes.push_back(new GlobalMesh{});
	const auto pCamera = new Camera{};
	const auto pTimer = new Timer();
	std::vector<GlobalMesh*> pMeshes{};

	//Initialize renders
	const auto pHardwareRenderer = new HardwareRenderer(pWindow,pGlobalMeshes, pCamera, pCullmode);
	const auto pSoftwareRenderer = new SoftwareRenderer(pWindow, pGlobalMeshes, pCamera, pCullmode);

	pCamera->Initialize(static_cast<float>(width) / static_cast<float>(height), 45.f, { 0,0,0 });

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;

	PrintKeyInfo();

	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if (e.key.keysym.scancode == SDL_SCANCODE_F1)
				{
					isHardware = !isHardware;
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F2)
				{
					pHardwareRenderer->ToggleRotation();
					pSoftwareRenderer->ToggleRotation();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F3)
				{
					pHardwareRenderer->ToggleFireMesh();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F4)
				{
					pHardwareRenderer->CycleSampleStates();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F5)
				{
					pSoftwareRenderer->ToggleRenderMode();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F6)
				{
					pSoftwareRenderer->ToggleNormal();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F7)
				{
					pSoftwareRenderer->ToggleDepthBuffer();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F8)
				{
					pSoftwareRenderer->ToggleBoundingBox();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F9)
				{
					*pCullmode = static_cast<CullMode>((static_cast<int>(*pCullmode) + 1) % 3);
					pHardwareRenderer->CycleCullModes();

				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F10)
				{
					pHardwareRenderer->ToggleClearCollor();
					pSoftwareRenderer->ToggleClearCollor();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F11)
				{
					showFps = !showFps;
				}
				//Test for a key
				//if (e.key.keysym.scancode == SDL_SCANCODE_X)
				break;
			default: ;
			}
		}

		//--------- Update ---------
		if (isHardware)
		{
			pHardwareRenderer->Update(pTimer);
		}
		else
		{
			pSoftwareRenderer->Update(pTimer);
		}
		
		pCamera->Update(pTimer);

		//--------- Render ---------
		if (isHardware)
		{
			pHardwareRenderer->Render();
		}
		else
		{
			pSoftwareRenderer->Render();
		}

		//--------- Timer ---------
		pTimer->Update();
		if (showFps)
		{
			printTimer += pTimer->GetElapsed();
			if (printTimer >= 1.f)
			{
				printTimer = 0.f;
				std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
			}
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pHardwareRenderer;
	delete pSoftwareRenderer;
	delete pTimer;
	delete pCamera;
	delete pCullmode;

	for (const auto pgMesh : pGlobalMeshes)
	{
		delete pgMesh;
	}

	ShutDown(pWindow);
	return 0;
}