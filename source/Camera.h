#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <chrono>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle},
			aspectRatio{}
		{
		}

		Vector3 origin{};

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		float nearC = 0.1f;
		float farC = 100.f;

		Matrix invViewMatrix{};
		Matrix viewMatrix{};
		Matrix projectionMatrix{};

		void Initialize(float _aspectRatio, float _fovAngle = 90.f, const Vector3& _origin = {0.f,0.f,0.f})
		{
			fovAngle = _fovAngle;
			aspectRatio = _aspectRatio;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
			CalculateProjectionMatrix();
		}

		void CalculateViewMatrix()
		{
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right);
			invViewMatrix = Matrix
			{
				right,
				up,
				forward,
				origin
			};

			viewMatrix = invViewMatrix.Inverse();

			//TODO W1
			//ONB => invViewMatrix
			//Inverse(ONB) => ViewMatrix

			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, nearC, farC);
			//TODO W2

			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(const Timer* pTimer)
		{
			float deltaTime = pTimer->GetElapsed();
			constexpr float cameraSpeed{ 10.f };

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//Keyboard stuff
			const bool boostedSpeed = pKeyboardState[SDL_SCANCODE_LSHIFT] || pKeyboardState[SDL_SCANCODE_RSHIFT];
			float boostAmount{ 1.5f };
			if (boostedSpeed == false)
			{
				boostAmount = 1.f;
			}
			if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
			{
				origin += cameraSpeed * deltaTime * forward * boostAmount;
			}
			if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
			{
				origin -= cameraSpeed * deltaTime * forward * boostAmount;
			}
			if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
			{
				origin -= cameraSpeed * deltaTime * right * boostAmount;
			}
			if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
			{
				origin += cameraSpeed * deltaTime * right * boostAmount;
			}

			//Delta time too small for calculations
			if (deltaTime < 0.01f)
			{
				deltaTime = 0.01f;
			}
			//Mouse stuff
			if (mouseState == SDL_BUTTON_LEFT)
			{
				origin -= mouseY * deltaTime * forward;
				totalYaw -= cameraSpeed * deltaTime * mouseX * TO_RADIANS;
			}
			if (mouseState == SDL_BUTTON_RIGHT || mouseState == SDL_BUTTON_RMASK)
			{
				totalYaw -= cameraSpeed * deltaTime * mouseX * TO_RADIANS;
				totalPitch -= cameraSpeed * deltaTime * mouseY * TO_RADIANS;
			}
			if (mouseState == SDL_BUTTON_X2)
			{
				origin.y -= mouseY * cameraSpeed * deltaTime;
			}

			forward = Matrix::CreateRotation(totalPitch, totalYaw, 0.f).TransformVector(Vector3::UnitZ);

			//Update Matrices
			CalculateViewMatrix();
		}
		void SetFOVAngle(float _fov)
		{
			fovAngle = _fov;
			CalculateProjectionMatrix();
		}
		void SetAspectRatio(float _aspect)
		{
			aspectRatio = _aspect;
			CalculateProjectionMatrix();
		}
		private:
		float fovAngle{90.f};
		float aspectRatio;
		public:
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
	};
}
