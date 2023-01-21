#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "HardwareRenderer.h"
#include "Camera.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
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
		"DirectX - Lee Vangraefschepe 2DAEGD15N",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Create settings
	const auto pCullmode = new CullMode{ CullMode::None };

	std::vector<GlobalMesh*> pGlobalMeshes{};
	pGlobalMeshes.push_back(new GlobalMesh{});
	pGlobalMeshes.push_back(new GlobalMesh{});


	//Initialize "framework"
	const auto pCamera = new Camera{};
	const auto pTimer = new Timer();
	std::vector<GlobalMesh*> pMeshes{};

	const auto pHardwareRenderer = new HardwareRenderer(pWindow,pGlobalMeshes, pCamera, pCullmode);

	pCamera->Initialize(static_cast<float>(width) / static_cast<float>(height), 45.f, { 0,0,-50 });

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	bool showFps{};
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
				if (e.key.keysym.scancode == SDL_SCANCODE_F2)
				{
					pHardwareRenderer->ToggleRotation();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F3)
				{
					pHardwareRenderer->ToggleFireMesh();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F4)
				{
					pHardwareRenderer->CycleSampleStates();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F9)
				{
					*pCullmode = static_cast<CullMode>((static_cast<int>(*pCullmode) + 1) % 3);
					pHardwareRenderer->CycleCullModes();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F10)
				{
					pHardwareRenderer->ToggleClearCollor();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F11)
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
		pHardwareRenderer->Update(pTimer);
		pCamera->Update(pTimer);

		//--------- Render ---------
		pHardwareRenderer->Render();

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