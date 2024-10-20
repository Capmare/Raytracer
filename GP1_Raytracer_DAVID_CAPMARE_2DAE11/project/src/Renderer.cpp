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

#include <algorithm>

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



	Matrix CameraToWorld = camera.CalculateCameraToWorld();

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			const float xNDC = (2 * ((px + .5f) / m_Width) - 1) * m_AspectRatio * m_FOV;
			const float yNDC = (1 - 2 * ((py + .5f) / m_Height)) * m_FOV;
			Vector3 rayDirection = {xNDC,yNDC,1.0};

			Ray viewRay = {camera.origin,CameraToWorld.TransformVector(rayDirection).Normalized() };

			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay,closestHit);

			ColorRGB finalColor{};
			

			if (closestHit.didHit)
			{
				for (const Light& light : pScene->GetLights())
				{

					Vector3 LightDirection = LightUtils::GetDirectionToLight(light,closestHit.origin);
					float normalizedDistance = LightDirection.Normalize();
					Ray lightRay{ closestHit.origin + closestHit.normal * 0.0005f, LightDirection, 0.0001f, normalizedDistance };


					ColorRGB Radiance{1,1,1};
					float observedArea{1};
					ColorRGB BRDF{1,1,1};

					switch (camera.m_CurrentLightingMode)
					{
					case dae::Camera::LightingMode::ObservedArea:
						observedArea = Vector3::Dot(closestHit.normal, LightDirection);
						break;
					case dae::Camera::LightingMode::Radiance:
						Radiance = LightUtils::GetRadiance(light, closestHit.origin);
						break;
					case dae::Camera::LightingMode::BRDF:
						BRDF = materials[closestHit.materialIndex]->Shade(closestHit, LightDirection, -viewRay.direction);
						break;
					case dae::Camera::LightingMode::Combined:
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


					if (pScene->DoesHit(lightRay) && camera.m_bShadowEnabled)
					{

						finalColor *= 0.99f;
					
					}
					else
					{
						finalColor += Radiance * observedArea * BRDF;

					}

				}
				
			}
			
			//Update Color in Buffer
			finalColor.MaxToOne();
			
			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}


