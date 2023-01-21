#pragma once
#include <cstdint>
#include <vector>
#include <string>

#include "Camera.h"
#include "DataTypes.h"
#include "SoftwareTexture.h"
#include "GlobalDefinitions.h"

using namespace dae;

class SoftwareRenderer
{
public:
	enum RenderMode
	{
		ObservedArea,
		Diffuse,
		Specular,
		Combined
	};

	SoftwareRenderer(SDL_Window* pWindow, std::vector<GlobalMesh*>& pGlobalMeshes, Camera* pCamera, CullMode* pCullMode);
	~SoftwareRenderer();

	SoftwareRenderer(const SoftwareRenderer&) = delete;
	SoftwareRenderer(SoftwareRenderer&&) noexcept = delete;
	SoftwareRenderer& operator=(const SoftwareRenderer&) = delete;
	SoftwareRenderer& operator=(SoftwareRenderer&&) noexcept = delete;

	void Update(Timer* pTimer);
	void Render();
	void ToggleDepthBuffer() { m_IsDepthBuffer = !m_IsDepthBuffer; }
	void ToggleNormal() { m_IsNormal = !m_IsNormal; }
	void ToggleRotation() { m_IsRotating = !m_IsRotating; }
	void ToggleBoundingBox() { m_IsBoundingBox = !m_IsBoundingBox; }
	void ToggleRenderMode() { m_Rendermode = static_cast<RenderMode>((static_cast<int>(m_Rendermode) + 1) % 4); }
	void CycleCullModes();
	void ToggleClearCollor() { m_ClearColor = !m_ClearColor; }

	bool SaveBufferToImage() const;

private:
	SDL_Window* m_pWindow{};

	SDL_Surface* m_pFrontBuffer{ nullptr };
	SDL_Surface* m_pBackBuffer{ nullptr };
	uint32_t* m_pBackBufferPixels{};

	float* m_pDepthBufferPixels{};

	Camera* m_pCamera{};
	CullMode* m_pCullMode{};

	SoftwareTexture* m_pTexture{ nullptr };
	SoftwareTexture* m_pTextureGloss{ nullptr };
	SoftwareTexture* m_pTextureNormal{ nullptr };
	SoftwareTexture* m_pTextureSpecular{ nullptr };

	int m_Width{};
	int m_Height{};
	float m_AspectRatio{};

	bool m_IsRotating{ true };
	bool m_IsNormal{ true };
	bool m_IsDepthBuffer{ false };
	bool m_IsBoundingBox{ false };
	bool m_ClearColor{ true };
	RenderMode m_Rendermode{ RenderMode::Combined };

	std::vector<GlobalMesh*>& m_pGlobalMeshes;
	std::vector<Mesh> m_MeshesWorld{};
	size_t m_VerticesCount{};
	Vertex_In* m_VerticesWorld;
	Vertex_In* m_VerticesNDC;
	Vector2* m_VerticesScreenSpace;

	void LoadMesh(const std::string& path);
	void VertexTransformationWorldToNDCNew(Mesh& mesh) const;

	static bool IsVerticesInFrustrum(const Vertex_Out& vertex);
	void PixelShading(const Vertex_Out& v) const;
	void CalculateSpecular(const Vector3& sampledNormal, const Vector3& lightDirection, const Vertex_Out& v, const float shininess, ColorRGB& output) const;

	//Draw traingles by using the index
	void DrawTriangle(int i, bool swapVertices, const Mesh& mesh);

	//Find size to reserve
	size_t FindReserveSize() const;
};

