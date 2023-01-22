#include "pch.h"
#include "SoftwareRenderer.h"
#include "SWUtils.h"

SoftwareRenderer::SoftwareRenderer(SDL_Window* pWindow, std::vector<GlobalMesh*>& pGlobalMeshes, Camera* pCamera, CullMode* pCullMode)
	: m_pWindow(pWindow)
	, m_pTexture{ SoftwareTexture::LoadFromFile("Resources/vehicle_diffuse.png") }
	, m_pTextureNormal{ SoftwareTexture::LoadFromFile("Resources/vehicle_normal.png") }
	, m_pTextureSpecular{ SoftwareTexture::LoadFromFile("Resources/vehicle_specular.png") }
	, m_pTextureGloss{ SoftwareTexture::LoadFromFile("Resources/vehicle_gloss.png") }
	, m_pGlobalMeshes{ pGlobalMeshes }
	, m_pCamera{ pCamera }
	, m_pCullMode{ pCullMode }
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = static_cast<uint32_t*>(m_pBackBuffer->pixels);

	m_pDepthBufferPixels = new float[m_Width * m_Height];

	LoadMesh("Resources/vehicle.obj");

	//Reserve max size
	const size_t reserveSize = FindReserveSize();
	m_VerticesCount = reserveSize;
	m_VerticesNDC = new Vertex_In[reserveSize];
	m_VerticesWorld = new Vertex_In[reserveSize];
	m_VerticesScreenSpace = new Vector2[reserveSize];
}

SoftwareRenderer::~SoftwareRenderer()
{
	delete[] m_pDepthBufferPixels;
	delete[] m_VerticesNDC;
	delete[] m_VerticesWorld;
	delete[] m_VerticesScreenSpace;
	delete m_pTexture;
	delete m_pTextureGloss;
	delete m_pTextureNormal;
	delete m_pTextureSpecular;
}

void SoftwareRenderer::Update(const Timer* pTimer) const
{
	if (m_IsRotating)
	{
		for (const auto pgMesh : m_pGlobalMeshes)
		{
			constexpr float rotationSpeed = 1.f;
			*pgMesh->pWorldMatrix = Matrix::CreateRotationY((rotationSpeed * pTimer->GetElapsed())) * *pgMesh->pWorldMatrix;
		}
	}
}

void SoftwareRenderer::Render()
{
	//Clears background
	if (m_ClearColor)
	{
		SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, 26, 26, 26));
	}
	else
	{
		SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));
	}
	
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	const int nrPixels{ m_Width * m_Height };
	std::fill_n(m_pDepthBufferPixels, nrPixels, FLT_MAX);

	//Loop over every mesh
	for (Mesh& mesh : m_MeshesWorld)
	{
		//Create vertices from indices
		for (size_t i = 0; i < mesh.indices.size(); ++i)
		{
			m_VerticesWorld[i] = mesh.vertices[mesh.indices[i]];
		}

		//Set maximum read count
		m_VerticesCount = mesh.indices.size();

		//VertexTransformationWorldToNDC();
		VertexTransformationWorldToNDCNew(mesh);

		for (size_t i = 0; i < m_VerticesCount; i++)
		{
			Vector2 temp;
			temp.x = (mesh.vertices_out[i].position.x + 1) / 2 * static_cast<float>(m_Width);
			temp.y = (1 - mesh.vertices_out[i].position.y) / 2 * static_cast<float>(m_Height);
			m_VerticesScreenSpace[i] = temp;
		}

		//RENDER LOGIC
		switch (mesh.primitiveTopology)
		{
		case PrimitiveTopology::TriangleList:
		{
			for (int i = 0; i < static_cast<int>(m_VerticesCount); i += 3)
			{
				DrawTriangle(i, false, mesh);
			}
		}
		break;
		case PrimitiveTopology::TriangleStrip:
		{
			for (int i = 0; i < static_cast<int>(m_VerticesCount) - 2; ++i)
			{
				DrawTriangle(i, i % 2, mesh);
			}
		}
		break;
		}
	}

	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, nullptr, m_pFrontBuffer, nullptr);
	SDL_UpdateWindowSurface(m_pWindow);
}

size_t SoftwareRenderer::FindReserveSize() const
{
	size_t max{};
	for (const Mesh& mesh : m_MeshesWorld)
	{
		max = std::max(max, mesh.indices.size());
	}
	return max;
}

void SoftwareRenderer::LoadMesh(const std::string& path)
{
	//Create empty mesh
	m_MeshesWorld.push_back(Mesh{ {},{}, PrimitiveTopology::TriangleList });

	//Load mesh
	Utils::SWParseOBJ(path, m_MeshesWorld[m_MeshesWorld.size() - 1].vertices, m_MeshesWorld[m_MeshesWorld.size() - 1].indices);

	//Set values for matrix
	const Vector3 translation = { Vector3{ 0.0f, 0.f, 50.f } };
	const Vector3 rotation = { 0.f, 0.f, 0.f };
	const Vector3 scale = { 1.f, 1.f, 1.f };

	//Generate and apply matrix
	for (const auto pgMesh : m_pGlobalMeshes)
	{
		*pgMesh->pWorldMatrix = Matrix::CreateScale(scale) * Matrix::CreateRotation(rotation) * Matrix::CreateTranslation(translation);
	}
}

void SoftwareRenderer::VertexTransformationWorldToNDCNew(Mesh& mesh) const
{
	const Matrix matrix = *m_pGlobalMeshes[0]->pWorldMatrix * m_pCamera->viewMatrix * m_pCamera->projectionMatrix;

	Vertex_Out v{};
	mesh.vertices_out.clear();
	mesh.vertices_out.reserve(m_VerticesCount);
	for (int i = 0; i < m_VerticesCount; i++)
	{
		v = { Vector4{}, m_VerticesWorld[i].color, m_VerticesWorld[i].uv, m_VerticesWorld[i].normal, m_VerticesWorld[i].tangent, m_VerticesWorld[i].viewDirection };

		//Apply tangent matrix
		v.tangent = m_pGlobalMeshes[0]->pWorldMatrix->TransformVector(v.tangent).Normalized();

		//Apply normal matrix
		v.normal = m_pGlobalMeshes[0]->pWorldMatrix->TransformVector(v.normal).Normalized();

		//Transfrom to camera matrix
		v.position = matrix.TransformPoint({ m_VerticesWorld[i].position, 1 });

		//Transform view direction
		v.viewDirection = matrix.TransformPoint(v.viewDirection).Normalized();

		//Perspective devide
		v.position.x /= v.position.w;
		v.position.y /= v.position.w;
		v.position.z /= v.position.w;

		mesh.vertices_out.emplace_back(v);
	}
}

bool SoftwareRenderer::IsVerticesInFrustrum(const Vertex_Out& vertex)
{
	if (vertex.position.x < -1.f || vertex.position.x > 1.f)
	{
		return false;
	}
	if (vertex.position.y < -1.f || vertex.position.y > 1.f)
	{
		return false;
	}
	if (vertex.position.z < 0.f || vertex.position.z > 1.f)
	{
		return false;
	}
	return true;
}

void SoftwareRenderer::PixelShading(const Vertex_Out& v) const
{
	const int pixelIndex = static_cast<int>(v.position.x) + (static_cast<int>(v.position.y) * m_Width);
	ColorRGB finalColor{ colors::White };

	if (m_IsDepthBuffer)
	{
		finalColor = { v.position.z,v.position.z ,v.position.z };
	}
	else
	{
		constexpr ColorRGB ambientColor{ 0.025f, 0.025f, 0.025f };
		const Vector3 lightDirection = { 0.577f, -0.577f , 0.577f };
		constexpr float lightIntensity = 7.f;
		constexpr float shininess = 25.f;
		constexpr float kd = 1.f;
		Vector3 sampledNormal = v.normal;

		if (m_IsNormal)
		{
			const Vector3 binormal = Vector3::Cross(v.normal, v.tangent);
			const Matrix tangentSpaceAxis = { v.tangent, binormal, v.normal, Vector3::Zero };

			sampledNormal = m_pTextureNormal->SampleToVector(v.uv);
			sampledNormal = 2.f * sampledNormal - Vector3::One;
			sampledNormal = tangentSpaceAxis.TransformVector(sampledNormal);

			sampledNormal.Normalize();
		}

		const float observedArea = Vector3::DotClamp(sampledNormal, -lightDirection);

		switch (m_Rendermode)
		{
		case ObservedArea:
			finalColor = colors::White * observedArea;
			break;
		case Diffuse:
			finalColor = (m_pTexture->Sample(v.uv) * kd / PI) * lightIntensity * observedArea;
			break;
		case Specular:
		{
			ColorRGB specularColor{ colors::Blue };
			CalculateSpecular(sampledNormal, lightDirection, v, shininess, specularColor);
			finalColor = specularColor * observedArea;
		}
		break;
		case Combined:
		{
			ColorRGB specularColor{};
			CalculateSpecular(sampledNormal, lightDirection, v, shininess, specularColor);

			finalColor = (m_pTexture->Sample(v.uv) * kd / PI) * lightIntensity * observedArea + specularColor;
		}
		break;
		}

		finalColor += ambientColor;
	}


	finalColor.MaxToOne();
	m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

void SoftwareRenderer::CalculateSpecular(const Vector3& sampledNormal, const Vector3& lightDirection, const Vertex_Out& v, const float shininess, ColorRGB& output) const
{
	const Vector3 reflectDirection{ Vector3::Reflect(lightDirection, sampledNormal) };

	const float cosAngle = Vector3::DotClamp(reflectDirection, -v.viewDirection);

	const float glossExponent{ m_pTextureGloss->Sample(v.uv).r * shininess };

	const float phong{ powf(cosAngle, glossExponent) };

	output = m_pTextureSpecular->Sample(v.uv) * phong;
}

void SoftwareRenderer::DrawTriangle(int i, bool swapVertices, const Mesh& mesh) const
{
	//Predefine indexes
	const uint32_t vertexIndex0 = i;
	const uint32_t vertexIndex1 = i + 1 * !swapVertices + 2 * swapVertices;
	const uint32_t vertexIndex2 = i + 2 * !swapVertices + 1 * swapVertices;

	//Calculate edges
	const Vector2 edgeV0V1 = m_VerticesScreenSpace[vertexIndex1] - m_VerticesScreenSpace[vertexIndex0];
	const Vector2 edgeV1V2 = m_VerticesScreenSpace[vertexIndex2] - m_VerticesScreenSpace[vertexIndex1];
	const Vector2 edgeV2V0 = m_VerticesScreenSpace[vertexIndex0] - m_VerticesScreenSpace[vertexIndex2];

	//Check if triangle is valid
	if (edgeV0V1.SqrMagnitude() < FLT_EPSILON || edgeV1V2.SqrMagnitude() < FLT_EPSILON || edgeV2V0.SqrMagnitude() < FLT_EPSILON)
	{
		return;
	}

	if (IsVerticesInFrustrum(mesh.vertices_out[vertexIndex0]) == false) { return; }
	if (IsVerticesInFrustrum(mesh.vertices_out[vertexIndex1]) == false) { return; }
	if (IsVerticesInFrustrum(mesh.vertices_out[vertexIndex2]) == false) { return; }

	const float fullTriangleArea = Vector2::Cross(edgeV0V1, edgeV1V2);

	//Create bounding box for optimized rendering
	Vector2 minBoundingBox{ Vector2::Min(m_VerticesScreenSpace[vertexIndex0], Vector2::Min(m_VerticesScreenSpace[vertexIndex1], m_VerticesScreenSpace[vertexIndex2])) };
	Vector2 maxBoundingBox{ Vector2::Max(m_VerticesScreenSpace[vertexIndex0], Vector2::Max(m_VerticesScreenSpace[vertexIndex1], m_VerticesScreenSpace[vertexIndex2])) };
	minBoundingBox.Clamp(static_cast<float>(m_Width), static_cast<float>(m_Height));
	maxBoundingBox.Clamp(static_cast<float>(m_Width), static_cast<float>(m_Height));
	constexpr int offset = 1;
	const int maxX = static_cast<int>(maxBoundingBox.x) + offset;
	const int maxY = static_cast<int>(maxBoundingBox.y) + offset;

	//Loop over every pixel that matches the bounding box
	for (int px{ static_cast<int>(minBoundingBox.x) }; px < maxX; ++px)
	{
		for (int py{ static_cast<int>(minBoundingBox.y) }; py < maxY; ++py)
		{
			const int index = px + py * m_Width;

			if (m_IsBoundingBox)
			{
				m_pBackBufferPixels[index] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(255),
					static_cast<uint8_t>(255),
					static_cast<uint8_t>(255));
				continue;
			}

			const Vector2 pointToSide = Vector2{ static_cast<float>(px), static_cast<float>(py) };
			const Vector2 pointV0 = pointToSide - m_VerticesScreenSpace[vertexIndex0];
			const Vector2 pointV1 = pointToSide - m_VerticesScreenSpace[vertexIndex1];
			const Vector2 pointV2 = pointToSide - m_VerticesScreenSpace[vertexIndex2];
			const float edge0 = Vector2::Cross(edgeV0V1, pointV0);
			const float edge1 = Vector2::Cross(edgeV1V2, pointV1);
			const float edge2 = Vector2::Cross(edgeV2V0, pointV2);

			//Culling
			const bool isFront{ edge0 >= 0 && edge1 >= 0 && edge2 >= 0 };
			const bool isBack{ edge0 <= 0 && edge1 <= 0 && edge2 <= 0 };
			if ((*m_pCullMode == Back && !isFront) || (*m_pCullMode == Front && !isBack) || (*m_pCullMode == None && !isBack && !isFront))
			{
				continue;
			}


			//Calculate the barycentric weight
			const float weightV0 = edge1 / fullTriangleArea;
			const float weightV1 = edge2 / fullTriangleArea;
			const float weightV2 = edge0 / fullTriangleArea;

			const float depthV0 = mesh.vertices_out[vertexIndex0].position.z;
			const float depthV1 = mesh.vertices_out[vertexIndex1].position.z;
			const float depthV2 = mesh.vertices_out[vertexIndex2].position.z;

			const float interpolatedDepth
			{
				1.0f /
				(weightV0 / depthV0 +
				weightV1 / depthV1 +
				weightV2 / depthV2)
			};

			if (m_pDepthBufferPixels[index] < interpolatedDepth)
			{
				continue;
			}

			m_pDepthBufferPixels[index] = interpolatedDepth;

			//Set basic info
			Vertex_Out pixelInfo{};
			pixelInfo.position.x = static_cast<float>(px);
			pixelInfo.position.y = static_cast<float>(py);
			pixelInfo.color = colors::White;

			if (m_IsDepthBuffer)
			{
				//Set depth for color
				pixelInfo.position.z = Remap(interpolatedDepth, .997f, 1.f);
			}
			else
			{
				//Calculate depth
				const float invInterpolatedDepthV0{ 1.f / mesh.vertices_out[vertexIndex0].position.w };
				const float invInterpolatedDepthV1{ 1.f / mesh.vertices_out[vertexIndex1].position.w };
				const float invInterpolatedDepthV2{ 1.f / mesh.vertices_out[vertexIndex2].position.w };

				const float interpolatedPixelDepth
				{
					1.f /
					(
						weightV0 * invInterpolatedDepthV0 +
						weightV1 * invInterpolatedDepthV1 +
						weightV2 * invInterpolatedDepthV2
					)
				};
				pixelInfo.position.w = interpolatedDepth;

				//Calculate uv
				const Vector2 pixelUV
				{
					(weightV0 * mesh.vertices_out[vertexIndex0].uv * invInterpolatedDepthV0 +
					weightV1 * mesh.vertices_out[vertexIndex1].uv * invInterpolatedDepthV1 +
					weightV2 * mesh.vertices_out[vertexIndex2].uv * invInterpolatedDepthV2)
						* interpolatedPixelDepth
				};
				pixelInfo.uv = pixelUV;

				//Calculate normal
				pixelInfo.normal =
				{
					((((mesh.vertices_out[vertexIndex0].normal / mesh.vertices_out[vertexIndex0].position.w) * weightV0) +
					((mesh.vertices_out[vertexIndex1].normal / mesh.vertices_out[vertexIndex1].position.w) * weightV1) +
					((mesh.vertices_out[vertexIndex2].normal / mesh.vertices_out[vertexIndex2].position.w) * weightV2)) * interpolatedPixelDepth).Normalized()
				};

				//Calculate tangent
				pixelInfo.tangent =
				{
					((((mesh.vertices_out[vertexIndex0].tangent / mesh.vertices_out[vertexIndex0].position.w) * weightV0) +
					((mesh.vertices_out[vertexIndex1].tangent / mesh.vertices_out[vertexIndex1].position.w) * weightV1) +
					((mesh.vertices_out[vertexIndex2].tangent / mesh.vertices_out[vertexIndex2].position.w) * weightV2)) * interpolatedPixelDepth).Normalized()
				};

				//Calculate viewDirection
				pixelInfo.viewDirection =
				{
					((((mesh.vertices_out[vertexIndex0].viewDirection / mesh.vertices_out[vertexIndex0].position.w) * weightV0) +
					((mesh.vertices_out[vertexIndex1].viewDirection / mesh.vertices_out[vertexIndex1].position.w) * weightV1) +
					((mesh.vertices_out[vertexIndex2].viewDirection / mesh.vertices_out[vertexIndex2].position.w) * weightV2)) * interpolatedPixelDepth).Normalized()
				};
			}
			PixelShading(pixelInfo);
		}
	}
}
