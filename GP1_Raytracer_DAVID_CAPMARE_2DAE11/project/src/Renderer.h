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
		void RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const struct Matrix& cameraToWorld, const struct Vector3& cameraOrigin) const;
		bool SaveBufferToImage() const;


	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		
		
		uint32_t* m_pBufferPixels{};



		int m_Width{};
		int m_Height{};
		float m_AspectRatio{};
		const float FOV_ANGLE{50};
		float m_FOV{};

	};
}
