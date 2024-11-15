#pragma once
#include <fstream>
#include "Maths.h"
#include <iostream>
#include "DataTypes.h"

#include <random>
#include "SquirellNoise5.hpp"
#include <immintrin.h>

//#define USE_SIMD_OP 


namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const float A{ ray.direction.SqrMagnitude() };
			const float B{ Vector3::Dot(2 * ray.direction, ray.origin - sphere.origin) };
			const float C{ (ray.origin - sphere.origin).SqrMagnitude() - sphere.radius * sphere.radius };

			const float discriminant{ B * B - 4 * A * C };

			hitRecord = HitRecord{};

			if (discriminant < .0f) return false;

			const float discriminantSqrt{ sqrt(discriminant) };
			float t{ (-B - discriminantSqrt) / (2 * A) };

			if (t < 0) t = (-B + discriminantSqrt) / (2 * A);
			if (t < ray.min || ray.max < t) return false;
			if (ignoreHitRecord) return true;

			hitRecord.t = t;
			hitRecord.origin = ray.origin + hitRecord.t * ray.direction;
			hitRecord.normal = (hitRecord.origin - sphere.origin) / sphere.radius;
			hitRecord.materialIndex = sphere.materialIndex;
			hitRecord.didHit = true;

			return hitRecord.didHit;

		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1
			hitRecord = HitRecord{};
			const float denominator{ Vector3::Dot(ray.direction, plane.normal) }; // dot is 0 if ray is parallel to the plane
			if (denominator < FLT_EPSILON && denominator > -FLT_EPSILON) return false; // if ray is parallel to the plane don't render

			const float t{ Vector3::Dot(plane.origin - ray.origin, plane.normal) / denominator };

			if (t > ray.min && t < ray.max)
			{
				if (ignoreHitRecord) return true;
				hitRecord.t = t;
				hitRecord.didHit = true;
				hitRecord.normal = plane.normal;
				hitRecord.origin = ray.origin + hitRecord.t * ray.direction;
				hitRecord.materialIndex = plane.materialIndex;
			}

			return hitRecord.didHit;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
#ifdef USE_SIMD_OP
		//TRIANGLE HIT-TESTS
		// https://stackoverflow.blog/2020/07/08/improving-performance-with-simd-intrinsics-in-three-use-cases/ thank god to this
		inline bool CalculateVerticesSIMD(const Vector3& P, const Vector3& n, const Vector3& v0, const Vector3& v1, const Vector3& v2) {
			
			// load all the points into vectors

			__m128 vecP = _mm_set_ps(0.0f, P.z, P.y, P.x);
			__m128 vecV0 = _mm_set_ps(0.0f, v0.z, v0.y, v0.x);
			__m128 vecV1 = _mm_set_ps(0.0f, v1.z, v1.y, v1.x);
			__m128 vecV2 = _mm_set_ps(0.0f, v2.z, v2.y, v2.x);
			__m128 vecN = _mm_set_ps(0.0f, n.z, n.y, n.x);
#pragma region 01
			// calculate e0 and p0
			__m128 e0 = _mm_sub_ps(vecV1, vecV0);
			__m128 p0 = _mm_sub_ps(vecP, vecV0);

			// cross product e0 and p0 //https://geometrian.com/programming/tutorials/cross-product/index.php
			__m128 e0p0_cross = _mm_sub_ps
			(
				_mm_mul_ps(_mm_shuffle_ps(e0, e0, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(p0, p0, _MM_SHUFFLE(3, 1, 0, 2))),
				_mm_mul_ps(_mm_shuffle_ps(e0, e0, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(p0, p0, _MM_SHUFFLE(3, 0, 2, 1)))
			);

			__m128 dot0 = _mm_dp_ps(e0p0_cross, vecN, 0x71);
			if (_mm_cvtss_f32(dot0) < 0) return false;
#pragma endregion 01
#pragma region 12

			__m128 e1 = _mm_sub_ps(vecV2, vecV1);
			__m128 p1 = _mm_sub_ps(vecP, vecV1);
			__m128 e1p1_cross = _mm_sub_ps(
				_mm_mul_ps(_mm_shuffle_ps(e1, e1, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(p1, p1, _MM_SHUFFLE(3, 1, 0, 2))),
				_mm_mul_ps(_mm_shuffle_ps(e1, e1, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(p1, p1, _MM_SHUFFLE(3, 0, 2, 1)))
			);
			__m128 dot1 = _mm_dp_ps(e1p1_cross, vecN, 0x71);
			if (_mm_cvtss_f32(dot1) < 0) return false;
#pragma endregion 12
#pragma  region 20
			__m128 e2 = _mm_sub_ps(vecV0, vecV2);
			__m128 p2 = _mm_sub_ps(vecP, vecV2);
			__m128 e2p2_cross = _mm_sub_ps(
				_mm_mul_ps(_mm_shuffle_ps(e2, e2, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(p2, p2, _MM_SHUFFLE(3, 1, 0, 2))),
				_mm_mul_ps(_mm_shuffle_ps(e2, e2, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(p2, p2, _MM_SHUFFLE(3, 0, 2, 1)))
			);
			__m128 dot2 = _mm_dp_ps(e2p2_cross, vecN, 0x71);
			if (_mm_cvtss_f32(dot2) < 0) return false;
#pragma endregion 20

			return true;



		}
#endif // USE_SIMD_OP

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			const Vector3 a = triangle.v1 - triangle.v0;
			const Vector3 b = triangle.v2 - triangle.v0;
			const Vector3 n = Vector3::Cross(a,b).Normalized();

			const float nv = Vector3::Dot(n, ray.direction);

			if (AreEqual(nv, 0)) return false;

			const int Direction = (ignoreHitRecord) ? 1 : -1;

			switch (triangle.cullMode)
			{
			case TriangleCullMode::FrontFaceCulling:
				if (nv * Direction > 0) { return false; }
				break;
			case TriangleCullMode::BackFaceCulling:
				if (nv * Direction < 0) { return false; }
				break;
			case TriangleCullMode::NoCulling:
				break;

			}

			const Vector3 L = triangle.v0 - ray.origin;

			const float t = Vector3::Dot(L,n) / nv;
			if (t < ray.min || t > ray.max) return false;

			const Vector3 P = ray.origin + ray.direction * t;

#ifdef USE_SIMD_OP
			if (!CalculateVerticesSIMD(P, n, triangle.v0, triangle.v1, triangle.v2)) return false;
#else
			const Vector3 e0 = triangle.v1 - triangle.v0;
			const Vector3 p0 = P - triangle.v0;
			if (Vector3::Dot(Vector3::Cross(e0, p0), n) < 0) return false;

			const Vector3 e1 = triangle.v2 - triangle.v1;
			const Vector3 p1 = P - triangle.v1;
			if (Vector3::Dot(Vector3::Cross(e1, p1), n) < 0) return false;

			const Vector3 e2 = triangle.v0 - triangle.v2;
			const Vector3 p2 = P - triangle.v2;
			if (Vector3::Dot(Vector3::Cross(e2, p2), n) < 0) return false;
#endif 
			 

			

			if (!ignoreHitRecord)
			{
				hitRecord.didHit = true;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.normal = n;
				hitRecord.origin = P;
				hitRecord.t = t;

			}

			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest

		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			float tx1 = (mesh.transformedMinAABB.x - ray.origin.x) / ray.direction.x;
			float tx2 = (mesh.transformedMaxAABB.x - ray.origin.x) / ray.direction.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			float ty1 = (mesh.transformedMinAABB.y - ray.origin.y) / ray.direction.y;
			float ty2 = (mesh.transformedMaxAABB.y - ray.origin.y) / ray.direction.y;

			tmin = std::min(tmin, std::min(ty1, ty2));
			tmax = std::max(tmax, std::max(ty1, ty2));

			float tz1 = (mesh.transformedMinAABB.z - ray.origin.z) / ray.direction.z;
			float tz2 = (mesh.transformedMaxAABB.z - ray.origin.z) / ray.direction.z;

			tmin = std::min(tmin, std::min(tz1, tz2));
			tmax = std::max(tmax, std::max(tz1, tz2));

			return (tmax > 0 && tmax >= tmin);
		}


		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{

			if (!SlabTest_TriangleMesh(mesh,ray))
			{
				return false;
			}

			Triangle CurrentTri{};
			HitRecord tempHit{};
			
			for (size_t idx{0}; idx < mesh.indices.size(); idx += 3)
			{
				
				CurrentTri = { mesh.transformedPositions[mesh.indices[idx]],mesh.transformedPositions[mesh.indices[idx + 1]],mesh.transformedPositions[mesh.indices[idx + 2]] };
				CurrentTri.normal = mesh.transformedNormals[mesh.indices[idx/3]];
				CurrentTri.cullMode = mesh.cullMode;
				CurrentTri.materialIndex = mesh.materialIndex;

				if (HitTest_Triangle(CurrentTri, ray, tempHit, ignoreHitRecord))
				{
					if (ignoreHitRecord)
					{
						return true;
					}

					if (tempHit.t < hitRecord.t)
					{
						hitRecord = tempHit;
					}
				}
			}


			return hitRecord.didHit;
		}

		

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}

		

#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			switch (light.type)
			{
			case LightType::Directional:
				return origin - light.direction;
			case LightType::Point:
				return light.origin - origin;
			}
			return Vector3();
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			switch (light.type)
			{
			case LightType::Point:
				return light.color * light.intensity / (target - light.origin).SqrMagnitude();
				break;
			case LightType::Directional:
				return light.color * light.intensity;
				break;

			}
			return ColorRGB();
		}
		static unsigned int i{};

		inline Vector3 GetRandomPointNearLight(const Light& light, const float& radius) {

			// uniform numbers in a sphere

			float u = Get1dNoiseZeroToOne(i);
			float theta = 2.0f * M_PI * Get1dNoiseZeroToOne(++i);
			float phi = acos(1.0f - 2.0f * u);

			// convert to cartesian coordinates
			float sinPhi = sin(phi);
			Vector3 randomPoint(
				sinPhi * cos(theta),
				sinPhi * sin(theta),
				cos(phi)
			);

			return light.origin + randomPoint * radius;

		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof())
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}