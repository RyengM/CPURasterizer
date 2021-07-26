#pragma once
#include "Math.h"
#include "Mesh.h"
#include "Texture.h"
#include "Light.h"
#include "FrameBuffer.h"
#include <functional>

namespace LearnTask
{
	struct V2F
	{
		// Common params
		Vec3 worldPos;
		Vec4 screenPos;
		Vec3 worldNormal;
		Vec3 color;
		Vec2 texCoord;
		// Normal mapping
		Vec3 T;
		Vec3 B;
		Vec3 N;
		// Shadow map
		Vec4 shadowCoord;
		// Mipmap
		Vec2 ddxy;

		static Vec2 BarycentricInterpolate(Vec2 attrA, Vec2 attrB, Vec2 attrC, float alpha, float beta, float gamma);
		static Vec3 BarycentricInterpolate(Vec3 attrA, Vec3 attrB, Vec3 attrC, float alpha, float beta, float gamma);
		static Vec4 BarycentricInterpolate(Vec4 attrA, Vec4 attrB, Vec4 attrC, float alpha, float beta, float gamma);

		static V2F Lerp(const V2F& v0, const V2F& v1, float t);
		static V2F LerpWorldSpace(const V2F& v0, const V2F& v1, float t);
	};

	struct V2FTriangle
	{
		V2F vertex[3];
	};

	struct Uniform
	{
		Vec3 eyePos;
		Mat4 model;
		Mat4 proj;
		Mat4 viewProj;
		Mat4 modelViewProj;
		Mat4 lightViewProj;
		DirectionalLight* dirLight;
		FrameBuffer* shadowMap;
	};

	struct VertexPayload
	{
		Vertex v;

		VertexPayload(const Vertex& v) : v(v) {};
	};

	struct FragmentPayload
	{
		V2F v2f;
		Texture* diffuseTex = nullptr;
		Texture* normalTex = nullptr;

		FragmentPayload(const V2F& v2f, Texture* texd, Texture* texn) : v2f(v2f),
						diffuseTex(texd), normalTex(texn) {};
	};

	class Shader;
	typedef V2F(Shader::*PVShader)(const VertexPayload&);
	typedef Vec4(Shader::*PFShader)(const FragmentPayload&);

	class Shader
	{
	public:
		Shader();
		Shader(std::string name) : name(name) {};

		V2F GouraudVertexShader(const VertexPayload& payload);
		Vec4 GouraudFragmentShader(const FragmentPayload& payload);

		V2F PhongVertexShader(const VertexPayload& payload);
		Vec4 PhongFragmentShader(const FragmentPayload& payload);

		V2F DefaultVertexShader(const VertexPayload& payload);
		Vec4 DefaultFragmentShader(const FragmentPayload& payload);

		float HardShadow(FrameBuffer* shadowMap, Vec4 texCoord, Vec3 normal, Vec3 lightDir);
		float PCF(FrameBuffer* shadowMap, Vec4 texCoord, Vec3 normal, Vec3 lightDir);
		float PCSS(FrameBuffer* shadowMap, Vec4 texCoord, Vec3 normal, Vec3 lightDir);
		float VSM(FrameBuffer* shadowMap, Vec4 texCoord, Vec3 normal, Vec3 lightDir);

		void SwitchShadow();

	public:
		std::string name;

		Uniform uniform;

		// 0: hard shadow, 1: PCF, 2: PCSS
		int shadowType = 0;
		bool bShadowSwitch = false;
		std::vector<std::string> shadowMode = { "HardShadow", "PCF", "PCSS" };

		PVShader activeVertexShader;
		PFShader activeFragmentShader;
		//std::function<V2F(VertexPayload)> activeVertexShader;
		//std::function<Vec4(FragmentPayload)> activeFragmentShader;
	};
}