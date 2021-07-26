#pragma once
#include "Camera.h"
#include "Collision.h"
#include "Material.h"
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

namespace LearnTask
{
	enum class ERenderMode {
		WireFrame = 0,
		Render = 1
	};

	enum class EWriteType {
		Render = 0,
		Depth = 1,
		Stencil = 2
	};

	struct ScanLineNode
	{
		// End position of the edge
		int endy;
		float endx;
		// Begin position of the edge, which has lower y
		float x;
		float y;
		float invSlope;
	};

	struct BarycentricResult
	{
		float alpha;
		float beta;
		float gamma;
	};

	class Rasterizer
	{
	public:
		Rasterizer();

		void Init();
		void Update();

		void BuildTextures();
		void BuildMeshes();
		void BuildMaterials();
		void BuildShaders();
		void BuildRenderItems();
		void BuildLight();

		void UpdateInfo();
		void ZPrepass();
		void ShadowPass();
		void RenderPass();

		void Draw(Shader* shader, const std::vector<RenderItem*>& items);

		void SwitchModel();
		void SwitchCullMode();

		// Clip
		EIntersectType CheckTriangleVisible(const V2FTriangle& tri);
		void SutherlandHodgeman(std::vector<V2FTriangle>& V2FTriangles, const V2FTriangle& tri);
		bool FaceCulling(const V2FTriangle& tri, int cull);
		bool InsidePlane(int PlaneIndex, const V2F& vertex);
		V2F IntersectPlane(int PlaneIndex, const V2F& last, const V2F& cur);

		// Rasterize
		void RasterTriangle(const V2FTriangle& tri);
		void ScanlineTriangle(const V2FTriangle& tri, const Shader* shader, const RenderItem* itemInfo, EWriteType type);

		// Draw pixel using barycentric interpolate
		void DrawPixel(int x, int y, const V2FTriangle& tri, const Shader* shader, const RenderItem* itemInfo);
		void DrawSubPixel(int x, int y, const V2FTriangle& tri);
		void DrawDepth(int x, int y, const V2FTriangle& tri);

		// Draw line
		void DrawLineDDA(const Vec3& begin, const Vec3& end, const V2FTriangle& tri, const Shader* shader, const RenderItem* itemInfo, EWriteType type);
		void DrawHorizonLineDDA(const Vec3& begin, const Vec3& end, const V2FTriangle& tri, const Shader* shader, const RenderItem* itemInfo, EWriteType type);
		void DrawLineBresenham(const Vertex& begin, const Vertex& end);
		void DrawLineTriangle(const V2FTriangle& tri);

		// Barycentric interpolate, get alpha, beta and gamma
		BarycentricResult ComputeBarycentric2D(float x, float y, const V2FTriangle& tri);
		bool CheckInTriangle(int x, int y, const V2FTriangle& tri);

		// Create world space tangent
		void CreateTangent(Mesh* mesh);

	public:
		std::shared_ptr<DeviceStatus> deviceStatus;

		std::unique_ptr<Camera> camera;
		std::unique_ptr<FrameBuffer> shadowBuffer;
		std::unique_ptr<FrameBuffer> frameBuffer;
		FrameBuffer* activeBuffer = nullptr;

		ERenderMode renderMode = ERenderMode::Render;

		bool bModelSwitching = false;
		bool bFaceCullSwitching = false;

		std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
		std::unordered_map<std::string, std::unique_ptr<Material>> materials;
		std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
		std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
		std::vector<std::unique_ptr<RenderItem>> renderItems;
		
		std::vector<RenderItem*> opaqueItems;
		std::vector<RenderItem*> lightItems;

		std::unique_ptr<DirectionalLight> dirLight;

		// Mesh switch switched
		std::vector<RenderItem*> candidateSwitchItems;
		int curModelNum = 0;
		int candidateItemStartPos;
		// Cull: 0 back cull, 1 front cull, 2 no cull
		int faceCull = 0;
		std::vector<std::string> cullMode = { "BackCull", "FrontCull", "None" };

	private:
		const Vec4 ClipPlanes[6] =
		{
			Vec4(0, 0, 1, 1),	// Near
			Vec4(0, 0, -1 ,1),	// Far
			Vec4(1, 0, 0, 1),	// Left
			Vec4(0, -1, 0, 1),	// Top
			Vec4(-1, 0, 0, 1),	// Right
			Vec4(0, 1 ,0, 1)	// Bottom
		};

		bool bRenderDepthOnly = false;
	};
}