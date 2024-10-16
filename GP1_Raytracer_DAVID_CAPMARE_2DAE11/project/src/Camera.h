#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Maths.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{
		}


		Vector3 origin{};
		float fovAngle{ 90.f };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		Matrix cameraToWorld{};

		enum class LightingMode
		{
			ObservedArea, //Lambert cosine
			Radiance, // IncidentRadiance
			BRDF, // Scattering of the light
			Combined // Observed area * radiance * BRDF
		};

		LightingMode m_CurrentLightingMode{ LightingMode::Combined };
		bool m_bShadowEnabled{ true };

		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();
			return {
			{right.x,right.y,right.z,0},
			{up.x,up.y,up.z,0},
			{forward.x,forward.y,forward.z,0},
			{origin.x,origin.y,origin.z,1}
			};
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//todo: W2
			//throw std::runtime_error("Not Implemented Yet");

			if (pKeyboardState[SDL_SCANCODE_F2])
			{
				m_bShadowEnabled = !m_bShadowEnabled;
			}

			if (pKeyboardState[SDL_SCANCODE_F3])
			{
				if (m_CurrentLightingMode == LightingMode::Combined)
				{
					m_CurrentLightingMode = LightingMode::ObservedArea;
				}
				else
				{
					m_CurrentLightingMode = static_cast<LightingMode>((int)m_CurrentLightingMode + 1);
				}
			}

			if (mouseState & SDL_BUTTON(3)) 
			{

				totalPitch -= mouseY * deltaTime;
				totalYaw -= mouseX * deltaTime;

				forward = Matrix::CreateRotation(Vector3(totalPitch * 4, totalYaw * 4,0)).TransformVector(Vector3::UnitZ);
				forward = forward.Normalized();

				

				if (pKeyboardState[SDL_SCANCODE_W])
				{
					origin += forward;
				}
				if (pKeyboardState[SDL_SCANCODE_S])
				{
					origin -= forward;
				}
				if (pKeyboardState[SDL_SCANCODE_A])
				{
					origin += Vector3::Cross(forward,up);
				}
				if (pKeyboardState[SDL_SCANCODE_D])
				{
					origin -= Vector3::Cross(forward, up);
				}
				if (pKeyboardState[SDL_SCANCODE_Q])
				{
					origin -= up;
				}
				if (pKeyboardState[SDL_SCANCODE_E])
				{
					origin += up;
				}

				
			}
		}
	};
}
