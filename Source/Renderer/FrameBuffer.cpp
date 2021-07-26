#include <Renderer/FrameBuffer.h>
#include <memory>
#include <cassert>

using namespace LearnTask;

FrameBuffer::FrameBuffer(int w, int h, bool bShadowMap)
{
	width = w, height = h;

	buffer = new unsigned char[width * height * channel];
	std::fill(buffer, buffer + width * height * channel, 0);
	
	depth = new float[width * height];
	std::fill(depth, depth + width * height, 1.f);

	if (msaaLevel > 1)
	{
		msaaBuffer = new unsigned char[msaaLevel * width * height * channel];
		std::fill(msaaBuffer, msaaBuffer + msaaLevel * width * height * channel, 0);
		msaaDepth = new float[msaaLevel * width * height];
		std::fill(msaaDepth, msaaDepth + msaaLevel * width * height, 1.f);
	}
	
	float viewportTrasform[16] = { width / 2.f, 0, 0, width / 2.f,
								   0, height / 2.f, 0, height / 2.f,
								   0, 0, 1, 0,
								   0, 0, 0, 1 };
	viewportMatrix = Mat4(viewportTrasform);
}

FrameBuffer::~FrameBuffer()
{
	delete buffer;
	buffer = nullptr;
	delete depth;
	depth = nullptr;

	if (msaaLevel > 1)
	{
		delete msaaBuffer;
		msaaBuffer = nullptr;
		delete msaaDepth;
		msaaDepth = nullptr;
	}
}

void FrameBuffer::ClearBuffer()
{
	std::fill(buffer, buffer + width * height * channel, 0);
	std::fill(depth, depth + width * height, 1.f);

	if (msaaLevel > 1)
	{
		std::fill(msaaBuffer, msaaBuffer + msaaLevel * width * height * channel, 0);
		std::fill(msaaDepth, msaaDepth + msaaLevel * width * height, 1.f);
	}
}

void FrameBuffer::SetPixel(int x, int y, const Vec3& color)
{
	assert(x >= 0 && x < width && y >= 0 && y < height);
	if (color.x > 1.f)		buffer[(y * width + x) * 4] = 255;
	else if (color.x < 0.f) buffer[(y * width + x) * 4] = 0;
	else					buffer[(y * width + x) * 4] = color.x * 255;
	if (color.y > 1.f)		buffer[(y * width + x) * 4 + 1] = 255;
	else if (color.y < 0.f)	buffer[(y * width + x) * 4 + 1] = 0;
	else					buffer[(y * width + x) * 4 + 1] = color.y * 255;
	if (color.z > 1.f)		buffer[(y * width + x) * 4 + 2] = 255;
	else if (color.z < 0.f)	buffer[(y * width + x) * 4 + 2] = 0;
	else					buffer[(y * width + x) * 4 + 2] = color.z * 255;
	buffer[(y * width + x) * 4 + 3] = 255;
}

float FrameBuffer::GetPixelDepth(int x, int y)
{
	if (x < 0 || x >= width || y < 0 || y >= height) return 1.f;
	return depth[y * width + x];
}

float FrameBuffer::GetPixelDepth(float x, float y)
{
	if (x < 0.f || x >= 1.f || y < 0.f || y >= 1.f)
		return 1.f;
	x *= width;
	y *= height;
	return depth[(int)y * width + (int)x];
}