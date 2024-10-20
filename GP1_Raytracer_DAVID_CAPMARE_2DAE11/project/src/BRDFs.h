#pragma once
#include "Maths.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			//todo: W3
			if (kd > 1)
			{
				kd = 1;
			}
			if (kd < 0)
			{
				kd = 0;
			}

			ColorRGB rho = cd * kd;
			

			return { rho / M_PI };
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			//todo: W3

			return {kd * cd / M_PI };
		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			//todo: W3

			Vector3 reflect = l - 2*(Vector3::Dot(n,l))*n;
			float cosAlpha = Vector3::Dot(reflect.Normalized(), v);
			

			return { ColorRGB(1,1,1) * ks * abs(powf(cosAlpha,exp)) };
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			//todo: W3
			
			ColorRGB Frgb = f0 + (ColorRGB(1, 1, 1) - f0) * powf((1 - (Vector3::Dot(h, v))), 5);

			return {Frgb};
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			//todo: W3
			float alphaSqr = roughness * roughness;
			alphaSqr *= alphaSqr;
			float nvec = Vector3::Dot(n, h);
			float nvec2 = nvec * nvec;

			float GGX = nvec2 * (alphaSqr - 1) + 1;
			GGX = M_PI * GGX * GGX;
			GGX = alphaSqr / GGX;
			return { GGX };
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			//todo: W3
			//todo: W3
			float nv = Vector3::Dot(n,v);
			float alphaSqr = (roughness * roughness);
			float k = ((alphaSqr + 1) * (alphaSqr + 1))/8 ;


			return {nv / ((nv * (1- k)) + k)};
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			//todo: W3

			return { GeometryFunction_SchlickGGX(n,v,roughness) * GeometryFunction_SchlickGGX(n,l,roughness) };
		}

	}
}