#pragma once

#include <cstdint>

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		bool SaveBufferToImage() const;

		void CycleLightingMode();
		void ToggleShadows() { m_bShadowEnabled = !m_bShadowEnabled; }

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		
		
		uint32_t* m_pBufferPixels{};

		enum class LightingMode
		{
			ObservedArea, //Lambert cosine
			Radiance, // IncidentRadiance
			BRDF, // Scattering of the light
			Combined // Observed area * radiance * BRDF
		};

		LightingMode m_CurrentLightingMode{LightingMode::Combined};

		int m_Width{};
		int m_Height{};
		float m_AspectRatio{};
		const float FOV_ANGLE{50};
		float m_FOV{};

		bool m_bShadowEnabled{false};
	};
}
