#include <Renderer/Rasterizer.h>
#include <Renderer/MeshGenerator.h>
#include <memory>
#include <cassert>
#include <algorithm>

using namespace LearnTask;

Rasterizer::Rasterizer()
{
	camera = std::make_unique<Camera>(Vec3(0, 0, -5), 45.f, 16.f / 9.f, 0.1f, 100.f);
	frameBuffer = std::make_unique<FrameBuffer>(1280, 720);
	shadowBuffer = std::make_unique<FrameBuffer>(1024, 1024, true);
	// Set framebuffer as default buffer
	activeBuffer = frameBuffer.get();

	deviceStatus = std::make_shared<DeviceStatus>();
	camera->deviceStatus = deviceStatus;
}

void Rasterizer::Init()
{
	BuildLight();
	BuildTextures();
	BuildMeshes();
	BuildMaterials();
	BuildShaders();
	BuildRenderItems();
}

void Rasterizer::BuildTextures()
{
	auto brick = std::make_unique<Texture>("../../Assets/Texture/brickwall.jpg");
	brick->name = "brick";
	textures[brick->name] = std::move(brick);

	auto brickN = std::make_unique<Texture>("../../Assets/Texture/brickwall_normal.jpg");
	brickN->name = "brickNormal";
	textures[brickN->name] = std::move(brickN);

	auto person = std::make_unique<Texture>("../../Assets/Texture/marina_diff.jpg");
	person->name = "person";
	textures[person->name] = std::move(person);

	auto personN = std::make_unique<Texture>("../../Assets/Texture/marina_normal.png");
	personN->name = "personNormal";
	textures[personN->name] = std::move(personN);
}

void Rasterizer::BuildMeshes()
{
	MeshLoader loader;
	Mesh cube = loader.LoadObj("../../Assets/Model/cube.obj");
	Mesh bunny = loader.LoadObj("../../Assets/Model/bunny.obj");
	Mesh person = loader.LoadObj("../../Assets/Model/marina.obj");

	auto cubeMesh = std::make_unique<Mesh>(cube);
	cubeMesh->name = "cube";
	meshes[cubeMesh->name] = std::move(cubeMesh);
	auto bunnyMesh = std::make_unique<Mesh>(bunny);
	bunnyMesh->name = "bunny";
	meshes[bunnyMesh->name] = std::move(bunnyMesh);
	auto personMesh = std::make_unique<Mesh>(person);
	personMesh->name = "person";
	meshes[personMesh->name] = std::move(personMesh);

	Mesh floor = MeshGenerator::CreateGrid(8.f, 8.f, 4, 4);
	auto floorMesh = std::make_unique<Mesh>(floor);
	floorMesh->name = "floor";
	meshes[floorMesh->name] = std::move(floorMesh);

	for (auto iter = meshes.begin(); iter != meshes.end(); ++iter)
	{
		if (!iter->second->bTangent)
		{
			CreateTangent(iter->second.get());
			iter->second->bTangent = true;
		}
	}
}

void Rasterizer::BuildMaterials()
{
	auto default = std::make_unique<Material>("default");
	default->diffuseTex = nullptr;
	default->normalTex = nullptr;
	materials[default->name] = std::move(default);

	auto brick = std::make_unique<Material>("brick");
	brick->diffuseTex = textures["brick"].get();
	brick->normalTex = textures["brickNormal"].get();
	materials[brick->name] = std::move(brick);

	auto person = std::make_unique<Material>("person");
	person->diffuseTex = textures["person"].get();
	person->normalTex = textures["personNormal"].get();
	materials[person->name] = std::move(person);
}

void Rasterizer::BuildShaders()
{
	auto phong = std::make_unique<Shader>("phong");
	/*phong->activeVertexShader = std::bind(&Shader::PhongVertexShader, phong.get(), std::placeholders::_1);
	phong->activeFragmentShader = std::bind(&Shader::PhongFragmentShader, phong.get(), std::placeholders::_1);*/
	phong->activeVertexShader = &Shader::PhongVertexShader;
	phong->activeFragmentShader = &Shader::PhongFragmentShader;
	shaders[phong->name] = std::move(phong);

	auto shadow = std::make_unique<Shader>("shadow");
	/*shadow->activeVertexShader = std::bind(&Shader::ShadowVertexShader, shadow.get(), std::placeholders::_1);
	shadow->activeFragmentShader = std::bind(&Shader::ShadowFragmentShader, shadow.get(), std::placeholders::_1);*/
	shadow->activeVertexShader = &Shader::DefaultVertexShader;
	shadow->activeFragmentShader = &Shader::DefaultFragmentShader;
	shaders[shadow->name] = std::move(shadow);

	auto plain = std::make_unique<Shader>("plain");
	plain->activeVertexShader = &Shader::DefaultVertexShader;
	plain->activeFragmentShader = &Shader::DefaultFragmentShader;
	shaders[plain->name] = std::move(plain);
}

void Rasterizer::BuildRenderItems()
{
	auto dirLightObj = std::make_unique<RenderItem>("dirLight");
	dirLightObj->position = dirLight->pos;
	dirLightObj->mesh = meshes["cube"].get();
	dirLightObj->mat = materials["default"].get();
	dirLightObj->scale = Vec3(0.02f);
	lightItems.push_back(dirLightObj.get());
	renderItems.push_back(std::move(dirLightObj));

	auto floor = std::make_unique<RenderItem>("floor");
	floor->position = Vec3(0.f, -1.f, 0.f);
	floor->scale = Vec3(0.4f);
	floor->mesh = meshes["floor"].get();
	floor->mat = materials["default"].get();
	opaqueItems.push_back(floor.get());
	renderItems.push_back(std::move(floor));

	candidateItemStartPos = renderItems.size();
	auto brick = std::make_unique<RenderItem>("cube");
	brick->mesh = meshes["cube"].get();
	brick->mat = materials["brick"].get();
	brick->position = Vec3(0.f, -0.5f, 0.f);
	brick->scale = Vec3(0.5f);
	opaqueItems.push_back(brick.get());
	candidateSwitchItems.push_back(brick.get());
	renderItems.push_back(std::move(brick));

	auto bunny = std::make_unique<RenderItem>("bunny");
	bunny->mesh = meshes["bunny"].get();
	bunny->mat = materials["default"].get();
	bunny->bDisplay = false;
	bunny->position = Vec3(0.f, -1.f, 0.f);
	bunny->scale = Vec3(0.5f);
	opaqueItems.push_back(bunny.get());
	candidateSwitchItems.push_back(bunny.get());
	renderItems.push_back(std::move(bunny));

	auto person = std::make_unique<RenderItem>("person");
	person->mesh = meshes["person"].get();
	person->mat = materials["person"].get();
	person->bDisplay = false;
	person->position = Vec3(0.f, -1.f, 0.f);
	person->scale = Vec3(0.001f);
	opaqueItems.push_back(person.get());
	candidateSwitchItems.push_back(person.get());
	renderItems.push_back(std::move(person));
}

void Rasterizer::BuildLight()
{
	dirLight = std::make_unique<DirectionalLight>();
	dirLight->power = Vec3(3.f);
	dirLight->distance = 3.f;
	dirLight->theta = 45.f;
	dirLight->phi = -45.f;
	dirLight->UpdateLightVectors();
	dirLight->deviceStatus = deviceStatus;
}

void Rasterizer::SwitchModel()
{
	if (!bModelSwitching)
	{
		candidateSwitchItems[(curModelNum) % candidateSwitchItems.size()]->bDisplay = false;
		candidateSwitchItems[(++curModelNum) % candidateSwitchItems.size()]->bDisplay = true;
		curModelNum %= candidateSwitchItems.size();
	}
}

void Rasterizer::SwitchCullMode()
{
	if (!bFaceCullSwitching)
		faceCull = (faceCull + 1) % 3;
}

void Rasterizer::Update()
{
	frameBuffer->ClearBuffer();
	shadowBuffer->ClearBuffer();

	UpdateInfo();

	// Z-prepass, it seems unnecessary when there is only one object, or maybe sort render items by distance

	// Z-hierarchy

	// Shadow pass
	ShadowPass();
	// Render pass
	RenderPass();
}

void Rasterizer::UpdateInfo()
{
	camera->UpdateProj();
	dirLight->UpdateLightVectors();

	for (int i = 0; i < renderItems.size(); ++i)
	{
		if (!renderItems[i]->bDisplay) continue;

		if (renderItems[i]->name == "dirLight")
			renderItems[i]->position = dirLight->pos;

		renderItems[i]->UpdateWorld();
	}
}

void Rasterizer::ShadowPass()
{
	activeBuffer = shadowBuffer.get();
	bRenderDepthOnly = true;
	auto shadow = shaders["shadow"].get();
	shadow->uniform.proj = Util::Ortho(-2.5f, 2.5f, -2.5f, 2.5f, 0.1f, 40.f);
	shadow->uniform.viewProj = shadow->uniform.proj * Util::LookAt(dirLight->pos, dirLight->pos + dirLight->dir, Vec3(1.f, 0.f, 0.f));
	Draw(shadow, opaqueItems);
	bRenderDepthOnly = false;
	activeBuffer = frameBuffer.get();
}

void Rasterizer::RenderPass()
{
	auto light = shaders["plain"].get();
	light->uniform.viewProj = camera->GetViewProjMatrix();
	Draw(light, lightItems);

	auto phong = shaders["phong"].get();
	phong->uniform.proj = camera->GetProjMatrix();
	phong->uniform.viewProj = camera->GetViewProjMatrix();
	phong->uniform.eyePos = camera->position;
	phong->uniform.dirLight = dirLight.get();
	phong->uniform.lightViewProj = shaders["shadow"]->uniform.viewProj;
	phong->uniform.shadowMap = shadowBuffer.get();
	Draw(phong, opaqueItems);
}

void Rasterizer::Draw(Shader* shader, const std::vector<RenderItem*>& items)
{
	for (int i = 0; i < items.size(); ++i)
	{
		if (!items[i]->bDisplay) continue;

		shader->uniform.model = items[i]->world;
		shader->uniform.modelViewProj = shader->uniform.viewProj * items[i]->world;

		std::vector<V2F> v2fs(items[i]->mesh->vertices.size());
		// Apply transform
		int v2fIdx = 0;
		for (const auto& vertex : items[i]->mesh->vertices)
		{
			VertexPayload payload(vertex);
			//V2F v2f = shader->activeVertexShader(payload);
			V2F v2f = (shader->*(shader->activeVertexShader))(payload);
			v2fs[v2fIdx++] = std::move(v2f);
		}

		// Per triangle operation
		for (int j = 0; j < items[i]->mesh->indices.size(); j += 3)
		{
			// Assemble triangle using vertex and index
			V2FTriangle tri;
			for (int k = 0; k < 3; ++k)
				tri.vertex[k] = v2fs[items[i]->mesh->indices[j + k]];
			// Clip
			std::vector<V2FTriangle> triangles;
			// Check status of triangle, only if the triangle is totally out of frustrum will be culled
			EIntersectType type = CheckTriangleVisible(tri);
			// If tiangle is out of view, discard
			if (type == EIntersectType::DISJOINT) continue;
			// If contain, just copy data
			else if (type == EIntersectType::CONTAIN)
				triangles.emplace_back(tri);
			// Intersect, clip triangle and re-assembly
			else
				SutherlandHodgeman(triangles, tri);
			// Rasterize
			if (renderMode == ERenderMode::WireFrame)
			{
				// Viewport transform
				// Dot not use viewport transform matrix, w is not equal to 1
				for (auto triangle : triangles)
				{
					for (auto& v2f : triangle.vertex)
					{
						v2f.screenPos.x /= v2f.screenPos.w;
						v2f.screenPos.y /= v2f.screenPos.w;
						v2f.screenPos.z /= v2f.screenPos.w;
						v2f.screenPos.x = 0.5f * activeBuffer->GetWidth() * (v2f.screenPos.x + 1.f);
						v2f.screenPos.y = 0.5f * activeBuffer->GetHeight() * (v2f.screenPos.y + 1.f);
						v2f.screenPos.z = v2f.screenPos.z * (camera->farPlane - 0.1f) / 2.f + (camera->farPlane + 0.1f) / 2.f;
					}
					DrawLineTriangle(triangle);
				}
			}
			else if (renderMode == ERenderMode::Render)
			{
				// Deal with clipped triangles
				for (auto triangle : triangles)
				{
					// Viewport transform
					// Dot not use viewport transform matrix, w is not equal to 1
					for (auto& v2f : triangle.vertex)
					{
						// Homogeneous division
						// We do not normalize w to 1, because we need use it later for persective correction
						v2f.screenPos.x /= v2f.screenPos.w;
						v2f.screenPos.y /= v2f.screenPos.w;
						v2f.screenPos.z /= v2f.screenPos.w;
						// Convert to screen space
						v2f.screenPos.x = 0.5f * activeBuffer->GetWidth() * (v2f.screenPos.x + 1.f);
						v2f.screenPos.y = 0.5f * activeBuffer->GetHeight() * (v2f.screenPos.y + 1.f);
						v2f.screenPos.z = v2f.screenPos.z * (camera->farPlane - 0.1f) / 2.f + (camera->farPlane + 0.1f) / 2.f;
					}
					// Face culling
					if (renderMode == ERenderMode::Render && faceCull != 2 && FaceCulling(triangle, faceCull)) continue;

					ScanlineTriangle(triangle, shader, items[i], EWriteType::Render);
				}
			}
		}
	}
}

EIntersectType Rasterizer::CheckTriangleVisible(const V2FTriangle& tri)
{
	bool b[3];
	memset(b, false, 3);

	if (tri.vertex[0].screenPos.w < camera->nearPlane && tri.vertex[1].screenPos.w < camera->nearPlane && tri.vertex[2].screenPos.w < camera->nearPlane)
		return EIntersectType::DISJOINT;
	if (tri.vertex[0].screenPos.w > camera->farPlane && tri.vertex[1].screenPos.w > camera->farPlane &&	tri.vertex[2].screenPos.w > camera->farPlane)
		return EIntersectType::DISJOINT;
	if (tri.vertex[0].screenPos.x < -tri.vertex[0].screenPos.w && tri.vertex[1].screenPos.x < -tri.vertex[1].screenPos.w && tri.vertex[2].screenPos.x < -tri.vertex[2].screenPos.w)
		return EIntersectType::DISJOINT;
	if (tri.vertex[0].screenPos.x > tri.vertex[0].screenPos.w && tri.vertex[1].screenPos.x > tri.vertex[1].screenPos.w && tri.vertex[2].screenPos.x > tri.vertex[2].screenPos.w)
		return EIntersectType::DISJOINT;
	if (tri.vertex[0].screenPos.y < -tri.vertex[0].screenPos.w && tri.vertex[1].screenPos.y < -tri.vertex[1].screenPos.w && tri.vertex[2].screenPos.y < -tri.vertex[2].screenPos.w)
		return EIntersectType::DISJOINT;
	if (tri.vertex[0].screenPos.y > tri.vertex[0].screenPos.w && tri.vertex[1].screenPos.y > tri.vertex[1].screenPos.w && tri.vertex[2].screenPos.y > tri.vertex[2].screenPos.w)
		return EIntersectType::DISJOINT;
	if (tri.vertex[0].screenPos.z < -tri.vertex[0].screenPos.w && tri.vertex[1].screenPos.z < -tri.vertex[1].screenPos.w && tri.vertex[2].screenPos.z < -tri.vertex[2].screenPos.w)
		return EIntersectType::DISJOINT;
	if (tri.vertex[0].screenPos.z > tri.vertex[0].screenPos.w && tri.vertex[1].screenPos.z > tri.vertex[1].screenPos.w && tri.vertex[2].screenPos.z > tri.vertex[2].screenPos.w)
		return EIntersectType::DISJOINT;

	for (int i = 0; i < 3; ++i)
	{
		if (tri.vertex[i].screenPos.w > camera->nearPlane && tri.vertex[i].screenPos.w < camera->farPlane &&
			tri.vertex[i].screenPos.x > -tri.vertex[i].screenPos.w && tri.vertex[i].screenPos.x < tri.vertex[i].screenPos.w &&
			tri.vertex[i].screenPos.y > -tri.vertex[i].screenPos.w && tri.vertex[i].screenPos.y < tri.vertex[i].screenPos.w &&
			tri.vertex[i].screenPos.z > -tri.vertex[i].screenPos.w && tri.vertex[i].screenPos.z < tri.vertex[i].screenPos.w)
			b[i] = true;
	}

	if (b[0] && b[1] && b[2]) return EIntersectType::CONTAIN;
	return EIntersectType::INTERSECT;
}

bool Rasterizer::InsidePlane(int PlaneIndex, const V2F& vertex)
{
	return vertex.screenPos.x * ClipPlanes[PlaneIndex].x + vertex.screenPos.y * ClipPlanes[PlaneIndex].y + vertex.screenPos.z * ClipPlanes[PlaneIndex].z + vertex.screenPos.w * ClipPlanes[PlaneIndex].w >= 0;
}

V2F Rasterizer::IntersectPlane(int PlaneIndex, const V2F& last, const V2F& cur)
{
	float dLast = last.screenPos.x * ClipPlanes[PlaneIndex].x + last.screenPos.y * ClipPlanes[PlaneIndex].y + last.screenPos.z * ClipPlanes[PlaneIndex].z + last.screenPos.w * ClipPlanes[PlaneIndex].w;
	float dCur = cur.screenPos.x * ClipPlanes[PlaneIndex].x + cur.screenPos.y * ClipPlanes[PlaneIndex].y + cur.screenPos.z * ClipPlanes[PlaneIndex].z + cur.screenPos.w * ClipPlanes[PlaneIndex].w;
	return V2F::LerpWorldSpace(last, cur, dLast / (dLast - dCur));
}

void Rasterizer::SutherlandHodgeman(std::vector<V2FTriangle>& triangles, const V2FTriangle& tri)
{
	std::vector<V2F> output = { tri.vertex[0], tri.vertex[1], tri.vertex[2] };
	// Deal with near plane
	{
		std::vector<V2F> input(output);
		output.clear();
		for (int j = 0; j < input.size(); ++j)
		{
			V2F cur = input[j];
			V2F last = input[(j + input.size() - 1) % input.size()];
			int curInside = cur.screenPos.w < camera->nearPlane ? -1 : 1;
			int lastInside = last.screenPos.w < camera->nearPlane ? -1 : 1;
			if (curInside * lastInside < 0)
			{
				float t = (last.screenPos.w - camera->nearPlane) / (last.screenPos.w - cur.screenPos.w);
				output.push_back(V2F::LerpWorldSpace(last, cur, t));
			}
			if (curInside > 0)
				output.push_back(cur);
		}
	}

	if (renderMode == ERenderMode::Render)
	{
		// Traverse for other planes
		for (int i = 1; i < 6; ++i)
		{
			std::vector<V2F> input(output);
			output.clear();
			// Deal with three vertices
			for (int j = 0; j < input.size(); ++j)
			{
				V2F cur = input[j];
				V2F last = input[(j + input.size() - 1) % input.size()];
				// Deal with other plane
				if (InsidePlane(i, cur))
				{
					if (!InsidePlane(i, last))
					{
						V2F intersect = IntersectPlane(i, last, cur);
						// Outer to innter
						output.push_back(intersect);
					}
					// All inner or outer to inner
					output.push_back(cur);
				}
				else if (InsidePlane(i, last))
				{
					V2F intersect = IntersectPlane(i, last, cur);
					// Inner to outer
					output.push_back(intersect);
				}
			}
		}
	}

	for (int i = 0; i < int(output.size()) - 2; ++i)
	{
		V2FTriangle t;
		t.vertex[0] = output[0];
		t.vertex[1] = output[i + 1];
		t.vertex[2] = output[i + 2];
		triangles.push_back(std::move(t));
	}
}

bool Rasterizer::FaceCulling(const V2FTriangle& tri, int cull)
{
	Vec3 p0 = tri.vertex[0].screenPos.xyz();
	Vec3 p1 = tri.vertex[1].screenPos.xyz();
	Vec3 p2 = tri.vertex[2].screenPos.xyz();

	Vec3 e0 = p1 - p0, e1 = p2 - p0;
	Vec3 dir = e0.Cross(e1).Normalize();
	Vec3 view(0, 0, 1);
	if (!cull)
		return dir.Dot(view) > 0.f;
	return dir.Dot(view) < 0.f;
}

//void Rasterizer::RasterTriangle(const V2FTriangle& tri)
//{
//	float minx = std::min(tri.vertex[0].screenPos.x, std::min(tri.vertex[1].screenPos.x, tri.vertex[2].screenPos.x));
//	float maxx = std::max(tri.vertex[0].screenPos.x, std::max(tri.vertex[1].screenPos.x, tri.vertex[2].screenPos.x));
//	float miny = std::min(tri.vertex[0].screenPos.y, std::min(tri.vertex[1].screenPos.y, tri.vertex[2].screenPos.y));
//	float maxy = std::max(tri.vertex[0].screenPos.y, std::max(tri.vertex[1].screenPos.y, tri.vertex[2].screenPos.y));
//
//	int left = std::max(int(minx), 0);
//	int bottom = std::max(int(miny), 0);
//	int right = std::min(int(maxx) + 1, activeBuffer->GetWidth() - 1);
//	int top = std::min(int(maxy) + 1, activeBuffer->GetHeight() - 1);
//
//	for (int x = left; x <= right; ++x)
//		for (int y = bottom; y <= top; ++y)
//			DrawPixel(x, y, tri);
//}

void Rasterizer::ScanlineTriangle(const V2FTriangle& tri, const Shader* shader, const RenderItem* itemInfo, EWriteType type)
{
	// Find vertex whose y coordinate is max or min
	int minY = INT_MAX;
	int maxY = -1;
	int minIndex = -1;
	int maxIndex = 10;
	for (int i = 0; i < 3; ++i)
	{
		if (tri.vertex[i].screenPos.y < minY)
		{
			minY = tri.vertex[i].screenPos.y;
			minIndex = i;
		}
		if (tri.vertex[i].screenPos.y > maxY)
		{
			maxY = tri.vertex[i].screenPos.y;
			maxIndex = i;
		}
	}
	int midIndex = 3 - minIndex - maxIndex;

	// Store info of two base edges, because this is scanline for triangle, it does not need linked list
	ScanLineNode lineVector[3];

	ScanLineNode node;
	node.endx = tri.vertex[midIndex].screenPos.x, node.endy = tri.vertex[midIndex].screenPos.y;
	node.x = tri.vertex[minIndex].screenPos.x, node.y = tri.vertex[minIndex].screenPos.y;
	node.invSlope = (tri.vertex[midIndex].screenPos.x - tri.vertex[minIndex].screenPos.x) / (tri.vertex[midIndex].screenPos.y - tri.vertex[minIndex].screenPos.y);
	lineVector[0] = std::move(node);

	node.endx = tri.vertex[maxIndex].screenPos.x, node.endy = tri.vertex[maxIndex].screenPos.y;
	node.x = tri.vertex[minIndex].screenPos.x, node.y = tri.vertex[minIndex].screenPos.y;
	node.invSlope = (tri.vertex[maxIndex].screenPos.x - tri.vertex[minIndex].screenPos.x) / (tri.vertex[maxIndex].screenPos.y - tri.vertex[minIndex].screenPos.y);
	lineVector[1] = std::move(node);

	// Scan line
	int drawIndex = 0;
	for (int y = minY; y <= maxY; ++y)
	{
		// To avoid numerical accuracy problem, clamp result to make sure line is in triangle
		float beginx = Util::Clamp(lineVector[drawIndex].x + (y - lineVector[drawIndex].y) * lineVector[drawIndex].invSlope, lineVector[drawIndex].x, lineVector[drawIndex].endx);
		float endx = Util::Clamp(lineVector[drawIndex + 1].x + (y - lineVector[drawIndex + 1].y) * lineVector[drawIndex + 1].invSlope, lineVector[drawIndex + 1].x, lineVector[drawIndex + 1].endx);
		DrawHorizonLineDDA(Vec3(beginx, y, 1), Vec3(endx, y, 1), tri, shader, itemInfo, type);

		// Update the third edge
		if (lineVector[0].endy == y)
		{
			ScanLineNode node;
			node.endx = tri.vertex[maxIndex].screenPos.x, node.endy = tri.vertex[maxIndex].screenPos.y;
			node.x = tri.vertex[midIndex].screenPos.x, node.y = tri.vertex[midIndex].screenPos.y;
			node.invSlope = (tri.vertex[maxIndex].screenPos.x - tri.vertex[midIndex].screenPos.x) / (tri.vertex[maxIndex].screenPos.y - tri.vertex[midIndex].screenPos.y);
			lineVector[2] = std::move(node);
			drawIndex = 1;
		}
	}
}

BarycentricResult Rasterizer::ComputeBarycentric2D(float x, float y, const V2FTriangle& tri)
{
	Vec3d a(tri.vertex[0].screenPos.x, tri.vertex[0].screenPos.y, tri.vertex[0].screenPos.z);
	Vec3d b(tri.vertex[1].screenPos.x, tri.vertex[1].screenPos.y, tri.vertex[1].screenPos.z);
	Vec3d c(tri.vertex[2].screenPos.x, tri.vertex[2].screenPos.y, tri.vertex[2].screenPos.z);

	double alpha = (x * (b.y - c.y) + y * (c.x - b.x) + b.x * c.y - c.x * b.y) /
				  (a.x * (b.y - c.y) + a.y * (c.x - b.x) + b.x * c.y - c.x * b.y);
	double beta =  (x * (c.y - a.y) + y * (a.x - c.x) + c.x * a.y - a.x * c.y) /
				  (b.x * (c.y - a.y) + b.y * (a.x - c.x) + c.x * a.y - a.x * c.y);
	double gamma = (x * (a.y - b.y) + y * (b.x - a.x) + a.x * b.y - b.x * a.y) /
				  (c.x * (a.y - b.y) + c.y * (b.x - a.x) + a.x * b.y - b.x * a.y);
	return { (float)alpha, (float)beta, (float)gamma };
}

bool Rasterizer::CheckInTriangle(int x, int y, const V2FTriangle& tri)
{
	Vec3 p(x, y, 1.f);
	Vec3 a(tri.vertex[0].screenPos.x, tri.vertex[0].screenPos.y, 1.f);
	Vec3 b(tri.vertex[1].screenPos.x, tri.vertex[1].screenPos.y, 1.f);
	Vec3 c(tri.vertex[2].screenPos.x, tri.vertex[2].screenPos.y, 1.f);

	Vec3 pa(a.x - p.x, a.y - p.y, a.z - p.z);
	Vec3 pb(b.x - p.x, b.y - p.y, b.z - p.z);
	Vec3 pc(c.x - p.x, c.y - p.y, c.z - p.z);
	Vec3 ab(b.x - a.x, b.y - a.y, b.z - a.z);
	Vec3 bc(c.x - b.x, c.y - b.y, c.z - b.z);
	Vec3 ca(a.x - c.x, a.y - c.y, a.z - c.z);
	Vec3 check(pa.x * ab.y - pa.y * ab.x, pb.x * bc.y - pb.y * bc.x, pc.x * ca.y - pc.y * ca.x);
	if (check.x <= 0.f && check.y <= 0.f && check.z <= 0.f || check.x >= 0.f && check.y >= 0.f && check.z >= 0.f)
		return true;
	return false;
}

void Rasterizer::DrawPixel(int x, int y, const V2FTriangle& tri, const Shader* shader, const RenderItem* itemInfo)
{
	if (!CheckInTriangle(x, y, tri)) return;
	// Get braycentric coefficient in screen space
	BarycentricResult r = ComputeBarycentric2D(x, y, tri);
	float alpha = r.alpha;
	float beta = r.beta;
	float gamma = r.gamma;
	// Perspective correction
	// ws = ¦Á(1 / w0) + ¦Â(1 / w1) + ¦Ã(1 / w2), wReciprocal = 1 / ws
	float wReciprocal = 1.f / (alpha / tri.vertex[0].screenPos.w + beta / tri.vertex[1].screenPos.w + gamma / tri.vertex[2].screenPos.w);
	// Interpolate z and convert value to [0, 1]
	float zInterpolated;
	// Perspective
	if (shader->uniform.proj.At(3, 2) == 1)
	{
		zInterpolated = wReciprocal / camera->farPlane;
		zInterpolated = (1.f / zInterpolated - 1.f / camera->nearPlane) / (1.f / camera->farPlane - 1.f / camera->nearPlane);
	}
	// Ortho
	else
		zInterpolated = (tri.vertex[0].screenPos.z * alpha + tri.vertex[1].screenPos.z * beta + tri.vertex[2].screenPos.z * gamma) / camera->farPlane;
	// Depth test
	int ind = y * activeBuffer->GetWidth() + x;
	if (zInterpolated < activeBuffer->depth[ind])
	{
		activeBuffer->depth[ind] = zInterpolated;

		V2F interpolatedVertex;
		if (!bRenderDepthOnly)
		{
			// Get braycentric coefficient in world space
			alpha = wReciprocal / tri.vertex[0].screenPos.w * alpha;
			beta = wReciprocal / tri.vertex[1].screenPos.w * beta;
			gamma = wReciprocal / tri.vertex[2].screenPos.w * gamma;
			// Interpolate, Is = (¦Á(I0 / w0) + ¦Â(I1 / w1) + ¦Ã(I2 / w2)) / ws
			interpolatedVertex.worldPos = V2F::BarycentricInterpolate(tri.vertex[0].worldPos, tri.vertex[1].worldPos, tri.vertex[2].worldPos, alpha, beta, gamma);
			interpolatedVertex.color = V2F::BarycentricInterpolate(tri.vertex[0].color, tri.vertex[1].color, tri.vertex[2].color, alpha, beta, gamma);
			interpolatedVertex.worldNormal = V2F::BarycentricInterpolate(tri.vertex[0].worldNormal, tri.vertex[1].worldNormal, tri.vertex[2].worldNormal, alpha, beta, gamma);
			interpolatedVertex.texCoord = V2F::BarycentricInterpolate(tri.vertex[0].texCoord, tri.vertex[1].texCoord, tri.vertex[2].texCoord, alpha, beta, gamma);
			interpolatedVertex.T = V2F::BarycentricInterpolate(tri.vertex[0].T, tri.vertex[1].T, tri.vertex[2].T, alpha, beta, gamma);
			interpolatedVertex.B = V2F::BarycentricInterpolate(tri.vertex[0].B, tri.vertex[1].B, tri.vertex[2].B, alpha, beta, gamma);
			interpolatedVertex.N = V2F::BarycentricInterpolate(tri.vertex[0].N, tri.vertex[1].N, tri.vertex[2].N, alpha, beta, gamma);
			interpolatedVertex.shadowCoord = V2F::BarycentricInterpolate(tri.vertex[0].shadowCoord, tri.vertex[1].shadowCoord, tri.vertex[2].shadowCoord, alpha, beta, gamma);

			// ddx, ddy for mipmap level calculation
			Vec2 ddx, ddy;
			Vec2 ux = interpolatedVertex.texCoord, uy = interpolatedVertex.texCoord;
			bool flag = true;
			{
				if (CheckInTriangle(x + 1, y, tri))
					r = ComputeBarycentric2D(x + 1, y, tri);
				else if (CheckInTriangle(x - 1, y, tri))
					r = ComputeBarycentric2D(x - 1, y, tri);
				else flag = false;
				if (flag)
				{
					wReciprocal = 1.f / (r.alpha / tri.vertex[0].screenPos.w + r.beta / tri.vertex[1].screenPos.w + r.gamma / tri.vertex[2].screenPos.w);
					ux = V2F::BarycentricInterpolate(tri.vertex[0].texCoord, tri.vertex[1].texCoord, tri.vertex[2].texCoord, r.alpha / tri.vertex[0].screenPos.w, r.beta / tri.vertex[1].screenPos.w, r.gamma / tri.vertex[2].screenPos.w) * wReciprocal;
				}
			}
			flag = true;
			{
				if (CheckInTriangle(x, y + 1, tri))
					r = ComputeBarycentric2D(x, y + 1, tri);
				else if (CheckInTriangle(x, y - 1, tri))
					r = ComputeBarycentric2D(x, y - 1, tri);
				else flag = false;
				if (flag)
				{
					wReciprocal = 1.f / (r.alpha / tri.vertex[0].screenPos.w + r.beta / tri.vertex[1].screenPos.w + r.gamma / tri.vertex[2].screenPos.w);
					uy = V2F::BarycentricInterpolate(tri.vertex[0].texCoord, tri.vertex[1].texCoord, tri.vertex[2].texCoord, r.alpha / tri.vertex[0].screenPos.w, r.beta / tri.vertex[1].screenPos.w, r.gamma / tri.vertex[2].screenPos.w) * wReciprocal;
				}
			}
			ddx = ux - interpolatedVertex.texCoord, ddy = uy - interpolatedVertex.texCoord;
			interpolatedVertex.ddxy = Vec2(ddx.Dot(ddx), ddy.Dot(ddy));
		}
		
		FragmentPayload payload(interpolatedVertex, itemInfo->mat->diffuseTex, itemInfo->mat->normalTex);
		//Vec4 color = shader->activeFragmentShader(payload);
		Vec4 color = (const_cast<Shader*>(shader)->*(shader->activeFragmentShader))(payload);
		activeBuffer->SetPixel(x, y, color.xyz());
	}
}

//void Rasterizer::DrawDepth(int x, int y, const V2FTriangle& tri)
//{
//	if (!CheckInTriangle(x, y, tri)) return;
//	// Get braycentric coefficient in screen space
//	std::tuple<float, float, float> t = ComputeBarycentric2D(x, y, tri);
//	float alpha = std::get<0>(t);
//	float beta = std::get<1>(t);
//	float gamma = std::get<2>(t);
//	// Perspective correction
//	// ws = ¦Á(1 / w0) + ¦Â(1 / w1) + ¦Ã(1 / w2), wReciprocal = 1 / ws
//	float wReciprocal = 1.f / (alpha / tri.vertex[0].screenPos.w + beta / tri.vertex[1].screenPos.w + gamma / tri.vertex[2].screenPos.w);
//	float zInterpolated = (tri.vertex[0].screenPos.z / tri.vertex[0].screenPos.w * alpha + tri.vertex[1].screenPos.z / tri.vertex[1].screenPos.w * beta + tri.vertex[2].screenPos.z / tri.vertex[2].screenPos.w * gamma) * wReciprocal;
//	// Depth test
//	int ind = y * activeBuffer->GetWidth() + x;
//	if (zInterpolated < activeBuffer->depth[ind])
//		activeBuffer->depth[ind] = zInterpolated;
//}

void Rasterizer::DrawLineDDA(const Vec3& begin, const Vec3& end, const V2FTriangle& tri, const Shader* shader, const RenderItem* itemInfo, EWriteType type)
{
	int ax = begin.x, ay = begin.y;
	int bx = end.x, by = end.y;

	int lx = bx - ax, ly = by - ay;
	float step = abs(lx) > abs(ly) ? abs(lx) : abs(ly);
	float dx = lx / step, dy = ly / step;

	for (int i = 0; i <= step; ++i)
	{
		int x = ax + dx * i, y = ay + dy * i;
		// Prevent the case that endpoint of triangle is culled
		if (step == 0) 
		{
			x = ax, y = ay;
		}
		if (x < 0 || x >= activeBuffer->GetWidth() || y < 0 || y >= activeBuffer->GetHeight()) continue;
		activeBuffer->SetPixel(x, y, Vec3(1.f));
	}
}

void Rasterizer::DrawHorizonLineDDA(const Vec3& begin, const Vec3& end, const V2FTriangle& tri, const Shader* shader, const RenderItem* itemInfo, EWriteType type)
{
	int ax = std::min(begin.x, end.x), bx = std::max(begin.x, end.x);
	int y = begin.y;

	int step = bx - ax;

	for (int i = 0; i <= step; ++i)
	{
		int x = ax + i;
		// Prevent the case that endpoint of triangle is culled
		if (step == 0) x = ax;
		if (x < 0 || x >= activeBuffer->GetWidth() || y < 0 || y >= activeBuffer->GetHeight()) continue;
		DrawPixel(x, y, tri, shader, itemInfo);
	}
}

void Rasterizer::DrawLineTriangle(const V2FTriangle& tri)
{
	DrawLineDDA(tri.vertex[0].screenPos.xyz(), tri.vertex[1].screenPos.xyz(), tri, nullptr, nullptr, EWriteType::Render);
	DrawLineDDA(tri.vertex[1].screenPos.xyz(), tri.vertex[2].screenPos.xyz(), tri, nullptr, nullptr, EWriteType::Render);
	DrawLineDDA(tri.vertex[2].screenPos.xyz(), tri.vertex[0].screenPos.xyz(), tri, nullptr, nullptr, EWriteType::Render);
}

void Rasterizer::CreateTangent(Mesh* mesh)
{
	for (int i = 0; i < mesh->indices.size(); i += 3)
	{
		Triangle tri;
		tri.vertex[0] = mesh->vertices[mesh->indices[i]];
		tri.vertex[1] = mesh->vertices[mesh->indices[i + 1]];
		tri.vertex[2] = mesh->vertices[mesh->indices[i + 2]];

		Vec3 e0 = (tri.vertex[1].pos - tri.vertex[0].pos).xyz();
		Vec3 e1 = (tri.vertex[2].pos - tri.vertex[0].pos).xyz();

		Vec2 t0 = tri.vertex[1].texCoord - tri.vertex[0].texCoord;
		Vec2 t1 = tri.vertex[2].texCoord - tri.vertex[0].texCoord;

		float coeff = 1.f / (t0.x * t1.y - t0.y * t1.x);
		mesh->vertices[mesh->indices[i]].tangentU.x = (t1.y * e0.x - t0.y * e1.x) * coeff;
		mesh->vertices[mesh->indices[i]].tangentU.y = (t1.y * e0.y - t0.y * e1.y) * coeff;
		mesh->vertices[mesh->indices[i]].tangentU.z = (t1.y * e0.z - t0.y * e1.z) * coeff;
		mesh->vertices[mesh->indices[i]].tangentU = mesh->vertices[mesh->indices[i]].tangentU.Normalize();

		mesh->vertices[mesh->indices[i + 1]].tangentU = mesh->vertices[mesh->indices[i]].tangentU;
		mesh->vertices[mesh->indices[i + 2]].tangentU = mesh->vertices[mesh->indices[i]].tangentU;
	}
}