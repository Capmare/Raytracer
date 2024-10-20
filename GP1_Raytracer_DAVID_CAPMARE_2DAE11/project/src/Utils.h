#pragma once
#include <fstream>
#include "Maths.h"
#include "DataTypes.h"

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
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			Vector3 a = triangle.v1 - triangle.v0;
			Vector3 b = triangle.v2 - triangle.v0;
			Vector3 n = Vector3::Cross(a,b).Normalized();

			float nv = Vector3::Dot(n, ray.direction);

			if ((nv* nv) < (FLT_EPSILON* FLT_EPSILON) ) return false;

			int Direction = (ignoreHitRecord) ? 1 : -1;

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

			Vector3 L = (ray.origin - triangle.v0 );

			float t = Vector3::Dot(L,n) / Vector3::Dot(ray.direction,n);
			if (t < ray.min || t > ray.max) return false;

			Vector3 P = ray.origin + ray.direction * t;

			Vector3 e0 = triangle.v1 - triangle.v0; 
			Vector3 p0 = P - triangle.v0;
			if (Vector3::Dot(Vector3::Cross(e0, p0),n) < 0) return false;

			Vector3 e1 = triangle.v2 - triangle.v1;
			Vector3 p1 = P - triangle.v1;
			if (Vector3::Dot(Vector3::Cross(e1, p1), n) < 0) return false;

			Vector3 e2 = triangle.v0 - triangle.v2;
			Vector3 p2 = P - triangle.v2;
			if (Vector3::Dot(Vector3::Cross(e2, p2), n) < 0) return false;

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
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			Triangle CurrentTri{};
			HitRecord tempHit;

			for (size_t idx{}; idx < mesh.indices.size(); idx += 3)
			{
				
				CurrentTri = {mesh.transformedPositions[mesh.indices[idx]],mesh.transformedPositions[mesh.indices[idx + 1]],mesh.transformedPositions[mesh.indices[idx + 2]] };
				CurrentTri.cullMode = mesh.cullMode;
				CurrentTri.materialIndex = mesh.materialIndex;
				CurrentTri.normal = mesh.transformedNormals[mesh.indices[idx/3]];

				if (ignoreHitRecord)
				{
					if (GeometryUtils::HitTest_Triangle(CurrentTri, ray))
					{
						return true;
					}
				}
				else
				{
					GeometryUtils::HitTest_Triangle(CurrentTri, ray, hitRecord);
					hitRecord = tempHit.t < hitRecord.t ? tempHit : hitRecord;
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