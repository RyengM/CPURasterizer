#include <Renderer/Shader.h>
#include <algorithm>

using namespace LearnTask;

#define PI2 6.283185307179586
#define PCF_NUM_SAMPLES 8
#define NUM_RINGS 10
#define SHADOW_HARDNESS 800

#define LIGHT_SIZE 2

Vec2 poissonDisk[PCF_NUM_SAMPLES];

float rand2to1(Vec2 uv) {
	const float a = 12.9898, b = 78.233, c = 43758.5453;
	float dt = uv.Dot(Vec2(a, b));
	float sn = sin(dt) * c;
	return sn - std::floor(sn);
}

void poissonDiskSamples(const Vec2 randomSeed) {

	float angleStep = PI2 * float(NUM_RINGS) / float(PCF_NUM_SAMPLES);
	float invNumSample = 1.0 / float(PCF_NUM_SAMPLES);

	float angle = rand2to1(randomSeed) * PI2;
	float radius = invNumSample;
	float radiusStep = radius;

	for (int i = 0; i < PCF_NUM_SAMPLES; i++) 
	{
		poissonDisk[i] = Vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
		radius += radiusStep;
		angle += angleStep;
	}
}

float blockerSearch(FrameBuffer* shadowMap, Vec3 coord)
{
	// Search in const area
	float totalDepth = 0;
	int blockerNum = 0;
	for (int i = 0; i < 25; i++) 
	{
		float depth = shadowMap->GetPixelDepth(coord.x + poissonDisk[i].x / shadowMap->GetWidth() * i, coord.y + poissonDisk[i].y / shadowMap->GetHeight() * i);
		if (depth < coord.z)
		{
			totalDepth += depth;
			blockerNum++;
		}
	}
	if (blockerNum == 0) return -1;

	return totalDepth / blockerNum;
}

float VSMBlockerSearch()
{
	return 0.f;
}

float penumbraEstimation(float avgDepth, float z)
{
	return (z - avgDepth) / avgDepth * LIGHT_SIZE + 1.f;
}

bool IsInShadowMap(FrameBuffer* shadowMap, Vec2 texCoord)
{
	if (texCoord.x < 0.f || texCoord.x >= 1.f || texCoord.y < 0.f || texCoord.y >= 1.f)
		return false;
	return true;
}

Shader::Shader()
{
	activeVertexShader = &Shader::PhongVertexShader;
	activeFragmentShader = &Shader::PhongFragmentShader;
	/*activeVertexShader = std::bind(&Shader::PhongVertexShader, this, std::placeholders::_1);
	activeFragmentShader = std::bind(&Shader::PhongFragmentShader, this, std::placeholders::_1);*/
}

float Shader::HardShadow(FrameBuffer* shadowMap, Vec4 texCoord, Vec3 normal, Vec3 lightDir)
{
	Vec3 coord = texCoord.xyz() / texCoord.w * 0.5f + 0.5f;
	if (!IsInShadowMap(shadowMap, coord.xy())) return 1.f;
	float depth = shadowMap->GetPixelDepth(coord.x, coord.y);

	float visibility = 0;
	float bias = std::max(5e-5, (1.0 - normal.Dot(lightDir)) * 5e-4);
	if (depth + bias > coord.z) visibility = 1.f;

	return visibility;
}

float Shader::PCF(FrameBuffer* shadowMap, Vec4 texCoord, Vec3 normal, Vec3 lightDir)
{
	Vec3 coord = texCoord.xyz() / texCoord.w * 0.5f + 0.5f;
	if (!IsInShadowMap(shadowMap, coord.xy())) return 1.f;
	poissonDiskSamples(coord.xy());

	float visibility = 0;
	for (int i = 0; i < PCF_NUM_SAMPLES; i++) 
	{
		float depth = shadowMap->GetPixelDepth(coord.x + poissonDisk[i].x / SHADOW_HARDNESS, coord.y + poissonDisk[i].y / SHADOW_HARDNESS);
		if (depth > coord.z) visibility += 1.f;
	}
	
	return visibility / PCF_NUM_SAMPLES;
}

float Shader::PCSS(FrameBuffer* shadowMap, Vec4 texCoord, Vec3 normal, Vec3 lightDir)
{
	Vec3 coord = texCoord.xyz() / texCoord.w * 0.5f + 0.5f;
	if (!IsInShadowMap(shadowMap, coord.xy())) return 1.f;
	poissonDiskSamples(coord.xy());

	float visibility = 0;
	
	float avgDepth = blockerSearch(shadowMap, coord);
	float sampleSize;
	if (avgDepth == -1.f)
		sampleSize = 1.f;
	else
		sampleSize = penumbraEstimation(avgDepth, coord.z);
	for (int i = 0; i < PCF_NUM_SAMPLES; i++) 
	{
		float depth = shadowMap->GetPixelDepth(coord.x + poissonDisk[i].x / SHADOW_HARDNESS * sampleSize, coord.y + poissonDisk[i].y / SHADOW_HARDNESS * sampleSize);
		if (depth > coord.z) visibility += 1.f;
	}

	return visibility / PCF_NUM_SAMPLES;
}

float Shader::VSM(FrameBuffer* shadowMap, Vec4 texCoord, Vec3 normal, Vec3 lightDir)
{
	//Vec3 coord = texCoord.xyz() / texCoord.w * 0.5f + 0.5f;
	//if (!IsInShadowMap(shadowMap, coord.xy())) return 1.f;

	//// Sample from filtered depth buffer
	//float M1;						// M1 = E[X]
	//float M2;						// M2 = E[X^2]
	//float variance = M2 - M1 * M1;	// ¦Ò^2 = E[X^2] - E^2[X]
	//float unoccluded = variance / (variance + (coord.z - M1) * (coord.z - M1));
	//float zOccAvg = (M1 - unoccluded * coord.z) / (1.f - unoccluded + 1e-5);
	//float visibility = variance / (variance + (coord.z - zOccAvg) * (coord.z - zOccAvg));
	//
	//return visibility;
	return 0.f;
}

V2F Shader::GouraudVertexShader(const VertexPayload& payload)
{
	V2F v2f;
	//v2f.screenPos = uniform.modelViewProj * payload.v.pos;
	//v2f.worldPos = (uniform.model * payload.v.pos).xyz();
	//v2f.texCoord = payload.v.texCoord;

	//Vec3 lightPos(-5, 5, -5);
	//float lightStrength = 10;

	//// WorldNormal = (M-1)T * normal
	//Vec3 n = (uniform.model.HomogeneousInverse().Transpose() * Vec4(Vec3(payload.v.normal).Normalize(), 1.f)).xyz().Normalize();
	//Vec3 l = (lightPos - v2f.worldPos).Normalize();
	//Vec3 v = (uniform.eyePos - v2f.worldPos).Normalize();
	//Vec3 h = (v + l).Normalize();

	//float distance = (lightPos - v2f.worldPos).Length();
	//float ambient = 0.2f;
	//float diffuse = lightStrength / distance * std::max(0.f, l.Dot(n));
	//Vec3 color = Util::Clamp(Vec3(ambient + diffuse), 0.f, 1.f);

	//v2f.color = color;
	return v2f;
}

Vec4 Shader::GouraudFragmentShader(const FragmentPayload& payload)
{
	Vec3 color = payload.v2f.color.x;
	if (payload.diffuseTex) color = payload.diffuseTex->Sample2D(payload.v2f.texCoord, payload.v2f.ddxy) * color.x;
	return Vec4(color, 1.f);
}

V2F Shader::PhongVertexShader(const VertexPayload& payload)
{
	V2F v2f;
	Mat4 model = uniform.model;
	v2f.screenPos = uniform.modelViewProj * payload.v.pos;
	v2f.worldPos = (model * payload.v.pos).xyz();
	// WorldNormal = (M-1)T * normal
	v2f.worldNormal = (model.HomogeneousInverse().Transpose() * Vec4(Vec3(payload.v.normal).Normalize(), 1.f)).xyz();
	v2f.color = payload.v.color;
	v2f.texCoord = payload.v.texCoord;
	v2f.T = (model * payload.v.tangentU).Normalize();
	v2f.N = (model * payload.v.normal).Normalize();
	// Re-orthogonalize T with respect to N
	v2f.T = (v2f.T - v2f.N * v2f.T.Dot(v2f.N)).Normalize();
	v2f.B = v2f.N.Cross(v2f.T);
	v2f.shadowCoord = uniform.lightViewProj * model * payload.v.pos;
	return v2f;
}

Vec4 Shader::PhongFragmentShader(const FragmentPayload& payload)
{
	Vec3 n;
	Vec3 worldNormal = Vec3(payload.v2f.worldNormal).Normalize();
	auto light = uniform.dirLight;
	if (payload.normalTex)
	{
		Vec3 normal = payload.normalTex->Sample2D(payload.v2f.texCoord, payload.v2f.ddxy);
		normal = (normal * 2.f - 1.f).Normalize();
		Mat3 TBN = Mat3(payload.v2f.T, payload.v2f.B, payload.v2f.N);
		n = (TBN * normal).Normalize();
	}
	else n = worldNormal;
	Vec3 l = -light->dir.Normalize();
	Vec3 v = (uniform.eyePos - payload.v2f.worldPos).Normalize();
	Vec3 h = (v + l).Normalize();

	Vec3 kd(1.f);
	if (payload.diffuseTex) kd = Util::Pow(payload.diffuseTex->Sample2D(payload.v2f.texCoord, payload.v2f.ddxy), 2.2f);
	float distance = (light->pos - payload.v2f.worldPos).Length();
	Vec3 ambient = kd * 0.2f;
	Vec3 diffuse = kd.ElementProduct(light->power) / distance * std::max(0.f, l.Dot(n));

	float visibility = 0.f;
	if (shadowType == 0)
		visibility = HardShadow(uniform.shadowMap, payload.v2f.shadowCoord, worldNormal, l);
	else if (shadowType == 1)
		visibility = PCF(uniform.shadowMap, payload.v2f.shadowCoord, worldNormal, l);
	else if (shadowType == 2)
		visibility = PCSS(uniform.shadowMap, payload.v2f.shadowCoord, worldNormal, l);
	Vec3 color = ambient + diffuse * visibility;
	float gamma = 1.f / 2.2f;
	Vec3 output(pow(color.x, gamma), pow(color.y, gamma), pow(color.z, gamma));

	return Vec4(output, 1.f);
}

V2F Shader::DefaultVertexShader(const VertexPayload& payload)
{
	V2F v2f;
	v2f.screenPos = uniform.modelViewProj * payload.v.pos;
	return v2f;
}

Vec4 Shader::DefaultFragmentShader(const FragmentPayload& payload)
{
	return Vec4(1.f);
}

void Shader::SwitchShadow()
{
	if (!bShadowSwitch)
		shadowType = (shadowType + 1) % 3;
}

Vec2 V2F::BarycentricInterpolate(Vec2 attrA, Vec2 attrB, Vec2 attrC, float alpha, float beta, float gamma)
{
	Vec2 ret;
	ret.x = attrA.x * alpha + attrB.x * beta + attrC.x * gamma;
	ret.y = attrA.y * alpha + attrB.y * beta + attrC.y * gamma;
	return ret;
}

Vec3 V2F::BarycentricInterpolate(Vec3 attrA, Vec3 attrB, Vec3 attrC, float alpha, float beta, float gamma)
{
	Vec3 ret;
	ret.x = attrA.x * alpha + attrB.x * beta + attrC.x * gamma;
	ret.y = attrA.y * alpha + attrB.y * beta + attrC.y * gamma;
	ret.z = attrA.z * alpha + attrB.z * beta + attrC.z * gamma;
	return ret;
}

Vec4 V2F::BarycentricInterpolate(Vec4 attrA, Vec4 attrB, Vec4 attrC, float alpha, float beta, float gamma)
{
	Vec4 ret;
	ret.x = attrA.x * alpha + attrB.x * beta + attrC.x * gamma;
	ret.y = attrA.y * alpha + attrB.y * beta + attrC.y * gamma;
	ret.z = attrA.z * alpha + attrB.z * beta + attrC.z * gamma;
	ret.w = attrA.w * alpha + attrB.w * beta + attrC.w * gamma;
	return ret;
}

V2F V2F::Lerp(const V2F& v0, const V2F& v1, float t)
{
	V2F v;

	float wReciprocal = 1.f / ((1.f - t) / v0.screenPos.w + t / v1.screenPos.w);
	v.screenPos = Vec4(v0.screenPos.xyz() * (1.f - t) + v1.screenPos.xyz() * t, wReciprocal);
	float alpha = wReciprocal / v0.screenPos.w * (1.f - t);
	float beta = wReciprocal / v1.screenPos.w * t;
	v.worldPos = v0.worldPos * alpha + v1.worldPos * beta;
	v.worldNormal = v0.worldNormal * alpha + v1.worldNormal * beta;
	v.texCoord = v0.texCoord * alpha + v1.texCoord * beta;
	v.color = v0.color * alpha + v1.color * beta;
	v.T = v0.T * alpha + v1.T * beta;
	v.B = v0.B * alpha + v1.B * beta;
	v.N = v0.N * alpha + v1.N * beta;
	v.shadowCoord = v0.shadowCoord * alpha + v1.shadowCoord * beta;
	
	return v;
}

V2F V2F::LerpWorldSpace(const V2F& v0, const V2F& v1, float t)
{
	V2F v;

	float alpha = 1.f - t;
	float beta = t;
	v.screenPos = v0.screenPos * alpha + v1.screenPos * t;
	v.worldPos = v0.worldPos * alpha + v1.worldPos * beta;
	v.worldNormal = v0.worldNormal * alpha + v1.worldNormal * beta;
	v.texCoord = v0.texCoord * alpha + v1.texCoord * beta;
	v.color = v0.color * alpha + v1.color * beta;
	v.T = v0.T * alpha + v1.T * beta;
	v.B = v0.B * alpha + v1.B * beta;
	v.N = v0.N * alpha + v1.N * beta;
	v.shadowCoord = v0.shadowCoord * alpha + v1.shadowCoord * beta;

	return v;
}