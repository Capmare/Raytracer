//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

#include <execution>
#include <algorithm>

#define PARALLEL_EXECUTION
#define SOFT_SHADOWS

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
	m_AspectRatio = (float)m_Width / (float)m_Height;
	m_FOV = tan((FOV_ANGLE * (M_PI / 180.f)) / 2.f);



}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();



	const Matrix cameraToWorld = camera.CalculateCameraToWorld();
	const uint32_t ammountOfPixels{ uint32_t(m_Width * m_Height) };

#ifdef PARALLEL_EXECUTION
	//parallel
	std::vector<uint32_t> pixelIndices{};
	pixelIndices.reserve(ammountOfPixels);
	for (uint32_t pixelIndex{}; pixelIndex < ammountOfPixels; ++pixelIndex) { pixelIndices.emplace_back(pixelIndex); }
	
	std::for_each(std::execution::par, pixelIndices.begin(), pixelIndices.end(), [&](int i) {
		RenderPixel(pScene, i, m_FOV, m_AspectRatio, cameraToWorld, camera.origin);
		});

#else
	//sync
	for (uint32_t pixelIndex{}; pixelIndex < ammountOfPixels; ++pixelIndex)
	{
		RenderPixel(pScene, pixelIndex, m_FOV, m_AspectRatio, cameraToWorld, camera.origin);
	}
#endif

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix cameraToWorld, const Vector3 cameraOrigin) const
{
	auto materials{ pScene->GetMaterials() };

	const uint32_t px{ pixelIndex % m_Width }, py{ pixelIndex / m_Width };

	const float rx{ px + 0.5f }, ry{ py + 0.5f };
	const float cx{ (2 * (rx / float(m_Width)) - 1) * aspectRatio * fov };
	const float cy{ (1 - (2 * (ry / float(m_Height)))) * fov };

	const Vector3 rayDirection = { cx,cy,1.0 };

	const Ray viewRay = { cameraOrigin,cameraToWorld.TransformVector(rayDirection).Normalized() };

	HitRecord closestHit{};
	pScene->GetClosestHit(viewRay, closestHit);

	ColorRGB finalColor{};


	if (closestHit.didHit)
	{
		for (const Light& light : pScene->GetLights())
		{

			Vector3 LightDirection = LightUtils::GetDirectionToLight(light, closestHit.origin);
			const float normalizedDistance = LightDirection.Normalize();
			const Ray lightRay{ closestHit.origin + closestHit.normal * 0.0005f, LightDirection, 0.0001f, normalizedDistance };


			ColorRGB Radiance{ 1,1,1 };
			float observedArea{ 1 };
			ColorRGB BRDF{ 1,1,1 };

			float shadowFactor = 1.0f;

#ifdef SOFT_SHADOWS

			const float lightRadius = .1f;
			const int numShadowSamples = 3;
			float totalWeight = 0.0f;

			for (int i = 0; i < numShadowSamples; ++i)
			{
				Vector3 randomizedLightPosition = LightUtils::GetRandomPointNearLight(light, lightRadius);

				Vector3 lightDirection = (randomizedLightPosition - closestHit.origin).Normalized();
				float distanceToLight = (randomizedLightPosition - closestHit.origin).Magnitude();
				Ray lightRay(closestHit.origin + closestHit.normal * 0.0005f, lightDirection, 0.0001f, distanceToLight);

				if (!pScene->DoesHit(lightRay))
				{
					shadowFactor += 1.0f;
				}


				float weight = exp(-0.5f * pow(distanceToLight / lightRadius, 2));
				shadowFactor += (pScene->DoesHit(lightRay) ? 0.0f : 1.0f) * weight;
				totalWeight += weight;

			}

			shadowFactor /= totalWeight;
			


#endif // SOFT_SHADOWS



			switch (pScene->m_CurrentLightingMode)
			{
			case LightingMode::ObservedArea:
				observedArea = Vector3::Dot(closestHit.normal, LightDirection);

				break;
			case LightingMode::Radiance:
				Radiance = LightUtils::GetRadiance(light, closestHit.origin);
				break;
			case LightingMode::BRDF:
				BRDF = materials[closestHit.materialIndex]->Shade(closestHit, LightDirection, -viewRay.direction);
				break;
			case LightingMode::Combined:
				BRDF = materials[closestHit.materialIndex]->Shade(closestHit, LightDirection, -viewRay.direction);
				Radiance = LightUtils::GetRadiance(light, closestHit.origin);
				observedArea = Vector3::Dot(closestHit.normal, LightDirection);
				break;
			default:
				break;
			}

			if (observedArea < 0)
			{
				continue;
			}

#ifdef SOFT_SHADOWS
			finalColor += Radiance * observedArea * BRDF * shadowFactor;
#else
			if (pScene->DoesHit(lightRay) && pScene->m_bShadowEnabled)
			{
				finalColor *= 1.f;
			}
			else
			{
				finalColor += Radiance * observedArea * BRDF * shadowFactor;
			}
#endif // _DEBUG
			

		}

	}
	
	//Update Color in Buffer
	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

