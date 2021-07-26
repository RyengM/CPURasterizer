#pragma once

namespace LearnTask
{
	class FrameBufferObject
	{
	public:
		FrameBufferObject(int width, int height) : width(width), height(height) {};

		FrameBufferObject(const FrameBufferObject& rhs) = delete;
		FrameBufferObject operator=(const FrameBufferObject& rhs) = delete;

		// Return frame buffer ID
		void BuildFrameBuffer();

		inline int GetWidth() { return width; };
		inline int GetHeight() { return height; };

		inline unsigned int GetFrameBufferID() { return frameBufferID; };
		inline unsigned int GetColorTextureID() { return colorTextrueID; };
		inline unsigned int GetDepthTextureID() { return depthTextrueID; };

	private:
		unsigned int frameBufferID = -1;
		unsigned int colorTextrueID = -1;
		unsigned int depthTextrueID = -1;
		int width = 0;
		int height = 0;
	};
}