#pragma once
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"
#include "Vector3.h"

namespace dae
{
	struct Vector2;

	class SoftwareTexture final
	{
	public:
		~SoftwareTexture()
		{
			if (m_pSurface)
			{
				SDL_FreeSurface(m_pSurface);
				m_pSurface = nullptr;
			}
		}

		static SoftwareTexture* LoadFromFile(const std::string& path)
		{
			//Load SDL_Surface using IMG_LOAD
			return new SoftwareTexture{ IMG_Load(path.c_str()) };
		}

		ColorRGB Sample(const Vector2& uv) const
		{
			//Set x & y for later usage
			const int x = static_cast<int>(uv.x * m_pSurface->w);
			const int y = static_cast<int>(uv.y * m_pSurface->h);

			//Prepare color get & calculate pixel index on texture
			const Uint32 pixel = m_pSurfacePixels[x + y * m_pSurface->w];
			Uint8 r{}, g{}, b{};

			//Get RGB color from texture
			SDL_GetRGB(pixel, m_pSurface->format, &r, &g, &b);

			//Convert to colorRGB
			constexpr float maxColorValue = 255.f;
			return ColorRGB{ r / maxColorValue, g / maxColorValue, b / maxColorValue };
		}

		Vector3 SampleToVector(const Vector2& uv) const
		{
			//Set x & y for later usage
			const int x = static_cast<int>(uv.x * m_pSurface->w);
			const int y = static_cast<int>(uv.y * m_pSurface->h);

			//Prepare color get & calculate pixel index on texture
			const Uint32 pixel = m_pSurfacePixels[x + y * m_pSurface->w];
			Uint8 r{}, g{}, b{};

			//Get RGB color from texture
			SDL_GetRGB(pixel, m_pSurface->format, &r, &g, &b);

			//Convert to colorRGB
			constexpr float maxColorValue = 255.f;
			return { r / maxColorValue, g / maxColorValue, b / maxColorValue };
		}

	private:
		//Constructor
		SoftwareTexture(SDL_Surface* pSurface) :
			m_pSurface{ pSurface },
			m_pSurfacePixels{ static_cast<uint32_t*>(pSurface->pixels) }
		{
		}

		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };
	};
}