#pragma once

#include "Math.h"
#include <string>

namespace LearnTask
{
	class Texture
	{
	public:
		Texture() = default;
		Texture(const std::string& filepath);
		~Texture();

		Texture(const Texture& tex);
		Texture(Texture&& tex);

		Texture& operator=(const Texture& tex);
		Texture& operator=(Texture&& tex);

		void Load(const std::string& filepath);

		Vec3 SamplePixel(int x, int y, int level);

		Vec3 Sample2D(const Vec2& texCoord, const Vec2& ddxy);

		inline int GetWidth() const noexcept { return width; }
		inline int GetHeight() const noexcept { return height; }
		inline int GetChannel() const noexcept { return channel; }

	public:
		std::string name;

	private:
		int width;
		int height;
		int channel = 4;

		// Mipmap, the size is 4/3 times bigger than origin size
		// Combine 4 channel data, each channel is 8 bit
		uint32_t** data;
		int mipmapLevel = 1;
	};
}