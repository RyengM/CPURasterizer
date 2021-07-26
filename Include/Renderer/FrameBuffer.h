#pragma once
#include "Mesh.h"
#include <tuple>

namespace LearnTask
{
	class FrameBuffer
	{
	public:
		FrameBuffer() = delete;
		FrameBuffer(int w, int h, bool bShadowMap = false);
		~FrameBuffer();

		void ClearBuffer();
		// Set pixels in screen space
		void SetPixel(int x, int y, const Vec3& color);

		float GetPixelDepth(int x, int y);
		// [0, 1]
		float GetPixelDepth(float x, float y);

		inline int GetWidth() const noexcept { return width; }
		inline int GetHeight() const noexcept { return height; }
		inline int GetChannel() const noexcept { return channel; }
		inline unsigned char* GetBuffer() const noexcept { return buffer; }
		inline Mat4 GetViewportMatrix() const noexcept { return viewportMatrix; }

	public:
		float* depth;

		int msaaLevel = 1;
		float* msaaDepth;

	private:
		int width = 1020;
		int height = 630;
		const int channel = 4;

		Mat4 viewportMatrix;
		unsigned char* buffer;
		
		// MSAA
		//int msaaLevel = 1;
		unsigned char* msaaBuffer;
	};
}