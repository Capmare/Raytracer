#include "Scene.h"
#include "Utils.h"
#include "Material.h"

namespace dae {

#pragma region Base Scene
	//Initialize Scene with Default Solid Color Material (RED)
	Scene::Scene() :
		m_Materials({ new Material_SolidColor({1,0,0}) })
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	Scene::~Scene()
	{
		for (auto& pMaterial : m_Materials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}

		m_Materials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
	{
		//todo W1
		HitRecord tempHit{};

		for (const Sphere& spheres : m_SphereGeometries)
		{	
			GeometryUtils::HitTest_Sphere(spheres,ray, tempHit);
			closestHit = tempHit.t < closestHit.t ? tempHit : closestHit;
			
		}
		
		for (const Plane& planes : m_PlaneGeometries)
		{
			GeometryUtils::HitTest_Plane(planes, ray, tempHit);
			closestHit = tempHit.t < closestHit.t ? tempHit : closestHit;
		}
		for (const TriangleMesh& Meshes : m_TriangleMeshGeometries)
		{
			GeometryUtils::HitTest_TriangleMesh(Meshes, ray, tempHit);
			closestHit = tempHit.t < closestHit.t ? tempHit : closestHit;
		}
		//throw std::runtime_error("Not Implemented Yet");
	}

	bool Scene::DoesHit(const Ray& ray) const
	{
		//todo W2
		for (const Sphere& spheres : m_SphereGeometries)
		{
			 if (GeometryUtils::HitTest_Sphere(spheres, ray))
			 {
				return true;
			 }
			 
		}

		for (const Plane& planes : m_PlaneGeometries)
		{
			if (GeometryUtils::HitTest_Plane(planes, ray))
			{
				return true;
			}
		}
		for (const Triangle& triangles : m_Triangles)
		{
			if (GeometryUtils::HitTest_Triangle(triangles,ray))
			{
				return true;
			}
		}

		for (const TriangleMesh& Meshes : m_TriangleMeshGeometries)
		{
			if (GeometryUtils::HitTest_TriangleMesh(Meshes, ray))
			{
				return true;
			}
		}

		return false;
	}

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& origin, float radius, unsigned char materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal;
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterial(Material* pMaterial)
	{
		m_Materials.push_back(pMaterial);
		return static_cast<unsigned char>(m_Materials.size() - 1);
	}
#pragma endregion
#pragma endregion

#pragma region SCENE W1
	void Scene_W1::Initialize()
	{
		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Spheres
		AddSphere({ -25.f, 0.f, 100.f }, 50.f, matId_Solid_Red);
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);

		//Plane
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);

		AddTriangleMesh(TriangleCullMode::NoCulling, matId_Solid_Blue);
	}
#pragma endregion


#pragma region SCENE W2
	void Scene_W2::Initialize()
	{
		m_Camera.origin = { .0f, 3.f, -9.f };
		m_Camera.fovAngle = { 45.f };

		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });
		const unsigned char matId_Solid_White = AddMaterial(new Material_SolidColor{ colors::White });

		//Plane
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 10 }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);

		//Light
		AddPointLight({ .0f, 5.f, -5.f }, 70.f, colors::White);
	}
#pragma endregion

	void Scene_W3::Initialize()
	{
		m_Camera.origin = { .0f, 1.f, -5.f };
		m_Camera.fovAngle = { 45.f };
		
		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .1f));
		
		const auto matLamber_GrayBlue = AddMaterial(new Material_Lambert({ .49f,.57f,.57f }, 1.f));
		// 
		AddPlane(Vector3{ 0.f,0.f,10.f }, Vector3{ 0.f,0.f,-1.f }, matLamber_GrayBlue);
		AddPlane(Vector3{ 0.f,0.f,0.f }, Vector3{ 0.f,1.f,0.f }, matLamber_GrayBlue);
		AddPlane(Vector3{ 0.f,10.f,0.f }, Vector3{ 0.f,-1.f,0.f }, matLamber_GrayBlue);
		AddPlane(Vector3{5.f,0.f,0.f}, Vector3{-1.f,0.f,0.f}, matLamber_GrayBlue);
		AddPlane(Vector3{-5.f,0.f,0.f}, Vector3{1.f,0.f,0.f}, matLamber_GrayBlue);
		 
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);
		
		
		AddPointLight({ .0f, 5.f, 5.f }, 50.f, ColorRGB{1.f,.61f,.45f});
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{1.f,.8f,.45f});
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{.34f,.47f,.68f});
	}



	void Scene_W4::Initialize()
	{


		m_Camera.origin = { .0f, 4.f, -10.f };
		m_Camera.fovAngle = 45;
		//m_Triangles.reserve(200);
		const Matrix rotation{ Matrix::CreateRotation(m_Camera.totalPitch, m_Camera.totalYaw, 0) };
		m_Camera.forward = rotation.TransformVector(Vector3::UnitZ).Normalized();



		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .1f));
		
		const auto matLamber_GrayBlue = AddMaterial(new Material_Lambert({ .49f,.57f,.57f }, 1.f));
		const auto matLamber_White = AddMaterial(new Material_Lambert(colors::White, 1.f));
		// 
		AddPlane(Vector3{ 0.f,0.f,10.f }, Vector3{ 0.f,0.f,-1.f }, matLamber_GrayBlue);
		AddPlane(Vector3{ 0.f,0.f,0.f }, Vector3{ 0.f,1.f,0.f }, matLamber_GrayBlue);
		AddPlane(Vector3{ 0.f,10.f,0.f }, Vector3{ 0.f,-1.f,0.f }, matLamber_GrayBlue);
		AddPlane(Vector3{ 5.f,0.f,0.f }, Vector3{ -1.f,0.f,0.f }, matLamber_GrayBlue);
		AddPlane(Vector3{ -5.f,0.f,0.f }, Vector3{ 1.f,0.f,0.f }, matLamber_GrayBlue);
		
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);
		
		const Triangle baseTriangle = { Vector3(-.75f,1.5f,0.f), Vector3(.75f,.0f,0.f),Vector3(-.75,0.f,0.f) };
		
		TriangleMesh* pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLamber_White);
		pMesh->AppendTriangle(baseTriangle, true);
		pMesh->Translate({ -1.75f, 4.5f, 0.f });
		pMesh->UpdateTransforms();
		m_meshes.push_back(pMesh);
		
		pMesh = AddTriangleMesh(TriangleCullMode::FrontFaceCulling, matLamber_White);
		pMesh->AppendTriangle(baseTriangle, true);
		pMesh->Translate({0.f, 4.5f, 0.f});
		pMesh->UpdateTransforms();
		m_meshes.push_back(pMesh);
		
		pMesh = AddTriangleMesh(TriangleCullMode::NoCulling, matLamber_White);
		pMesh->AppendTriangle(baseTriangle, true);
		pMesh->Translate({1.75f, 4.5f, 0.f});
		pMesh->UpdateTransforms();
		m_meshes.push_back(pMesh);
		
		AddPointLight({ .0f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f,.61f,.45f });
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f,.8f,.45f });
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f,.47f,.68f });
	}

	void Scene_W4::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);

		const float yawAngle{ ((cos(pTimer->GetTotal()) + 1.f) / 2.f * PI_2 * PI_2) };
		
		
		for (TriangleMesh* m : m_meshes)
		{
			m->RotateY(yawAngle);
			m->UpdateTransforms();
			m->UpdateAABB();
		}
	}

	void Scene_W4_Bunny::Initialize()
	{
		m_Camera.origin = { .0f, 1.f, -5.f };
		m_Camera.fovAngle = 45;
		//m_Triangles.reserve(200);
		const Matrix rotation{ Matrix::CreateRotation(m_Camera.totalPitch, m_Camera.totalYaw, 0) };
		m_Camera.forward = rotation.TransformVector(Vector3::UnitZ).Normalized();



		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .1f));

		const auto matLamber_GrayBlue = AddMaterial(new Material_Lambert({ .49f,.57f,.57f }, 1.f));
		const auto matLamber_White = AddMaterial(new Material_Lambert(colors::White, 1.f));
		// 
		AddPlane(Vector3{ 0.f,0.f,10.f }, Vector3{ 0.f,0.f,-1.f }, matLamber_GrayBlue);
		AddPlane(Vector3{ 0.f,0.f,0.f }, Vector3{ 0.f,1.f,0.f }, matLamber_GrayBlue);
		AddPlane(Vector3{ 0.f,10.f,0.f }, Vector3{ 0.f,-1.f,0.f }, matLamber_GrayBlue);
		AddPlane(Vector3{ 5.f,0.f,0.f }, Vector3{ -1.f,0.f,0.f }, matLamber_GrayBlue);
		AddPlane(Vector3{ -5.f,0.f,0.f }, Vector3{ 1.f,0.f,0.f }, matLamber_GrayBlue);

		m_mesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLamber_White);
		Utils::ParseOBJ("resources/lowpoly_bunny.obj", m_mesh->positions, m_mesh->normals, m_mesh->indices);


		AddPointLight({ .0f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f,.61f,.45f });
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f,.8f,.45f });
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f,.47f,.68f });

	}

	void Scene_W4_Bunny::Update(Timer* pTimer)
	{
		const float yawAngle{ ((cos(pTimer->GetTotal()) + 1.f) / 2.f * PI_2 * PI_2) };

		m_mesh->RotateY(yawAngle);
		m_mesh->UpdateTransforms();
		m_mesh->UpdateAABB();

	}

}
