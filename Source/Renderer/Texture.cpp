#include <Renderer/Texture.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <algorithm>

using namespace LearnTask;

Texture::Texture(const std::string& filepath)
{
	Load(filepath);
}

Texture::~Texture()
{
	for (int i = 0; i < mipmapLevel; ++i)
		delete data[i];
	delete data;
	data = nullptr;
}

Texture::Texture(const Texture& tex)
{
	width = tex.width;
	height = tex.height;
	channel = tex.channel;

	data = new uint32_t*[mipmapLevel];
	int mipmapWidth = width;
	int mipmapHeight = height;
	for (int i = 0; i < mipmapLevel; ++i)
	{
		data[i] = new uint32_t[mipmapWidth * mipmapHeight];
		memcpy(data[i], tex.data[i], mipmapWidth * mipmapHeight);
		mipmapWidth >>= 1;
		mipmapHeight >>= 1;
	}
}

Texture::Texture(Texture&& tex)
{
	width = tex.width;
	height = tex.height;
	channel = tex.channel;

	data = tex.data;
	tex.data = nullptr;
}

Texture& Texture::operator=(const Texture& tex)
{
	width = tex.width;
	height = tex.height;
	channel = tex.channel;

	data = new uint32_t*[mipmapLevel];
	int mipmapWidth = width;
	int mipmapHeight = height;
	for (int i = 0; i < mipmapLevel; ++i)
	{
		data[i] = new uint32_t[mipmapWidth * mipmapHeight];
		memcpy(data[i], tex.data[i], mipmapWidth * mipmapHeight);
		mipmapWidth >>= 1;
		mipmapHeight >>= 1;
	}
	return *this;
}

Texture& Texture::operator=(Texture&& tex)
{
	width = tex.width;
	height = tex.height;
	channel = tex.channel;

	data = tex.data;
	tex.data = nullptr;

	return *this;
}

void Texture::Load(const std::string& filepath)
{
	stbi_set_flip_vertically_on_load(true);
	int c;
	unsigned char* input = stbi_load(filepath.c_str(), &width, &height, &c, 0);

	if (!input)
	{
		std::cout << "Texture failed to load at path: " << filepath << std::endl;
		stbi_image_free(input);
	}

	mipmapLevel = log(width) / log(2) + 1;
	data = new uint32_t*[mipmapLevel];
	data[0] = new uint32_t[width * height];
	// RGBA
	for (int i = 0; i < width * height; ++i)
		data[0][i] = (input[i * c] << 24) | (input[i * c + 1] << 16) | (input[i * c + 2] << 8) | (c == 4 ? input[i * c + 3] : 0xff);
	// What if rectangle whose width is not equal to height?
	int mipmapWidth = width;
	int mipmapHeight = height;
	for (int level = 1; level < mipmapLevel; ++level)
	{
		mipmapWidth >>= 1;
		mipmapHeight >>= 1;

		// Alloc
		data[level] = new uint32_t[mipmapWidth * mipmapHeight];

		// The length of current mipmap
		int len = mipmapWidth * mipmapHeight;
		for (int i = 0; i < len; ++i)
		{
			int x = i % mipmapWidth;
			int y = i / mipmapHeight;

			uint32_t dataA = data[level - 1][y * 2 * 2 * mipmapWidth + x * 2];
			float r0 = (dataA >> 24) & 0xff;
			float g0 = (dataA >> 16) & 0xff;
			float b0 = (dataA >> 8) & 0xff;
			float a0 = dataA & 0xff;

			uint32_t dataB = data[level - 1][y * 2 * 2 * mipmapWidth + x * 2 + 1];
			float r1 = (dataB >> 24) & 0xff;
			float g1 = (dataB >> 16) & 0xff;
			float b1 = (dataB >> 8) & 0xff;
			float a1 = dataB & 0xff;

			uint32_t dataC = data[level - 1][(y * 2 + 1) * 2 * mipmapWidth + x * 2];
			float r2 = (dataC >> 24) & 0xff;
			float g2 = (dataC >> 16) & 0xff;
			float b2 = (dataC >> 8) & 0xff;
			float a2 = dataC & 0xff;

			uint32_t dataD = data[level - 1][(y * 2 + 1) * 2 * mipmapWidth + x * 2 + 1];
			float r3 = (dataD >> 24) & 0xff;
			float g3 = (dataD >> 16) & 0xff;
			float b3 = (dataD >> 8) & 0xff;
			float a3 = dataD & 0xff;

			uint32_t r = uint32_t((r0 + r1 + r2 + r3) / 4.f);
			uint32_t g = uint32_t((g0 + g1 + g2 + g3) / 4.f);
			uint32_t b = uint32_t((b0 + b1 + b2 + b3) / 4.f);
			uint32_t a = uint32_t((a0 + a1 + a2 + a3) / 4.f);

			data[level][y * mipmapWidth + x] = (r << 24) | (g << 16) | (b << 8) | a;
		}
	}

	stbi_image_free(input);
}

Vec3 Texture::SamplePixel(int x, int y, int level)
{
	int scale = Util::Pow(2, level);
	int w = width / scale;
	x /= scale, y /= scale;
	uint32_t color = data[level][y * w + x];
	float r = float(color >> 24 & 0xff) / 255.f;
	float g = float(color >> 16 & 0xff) / 255.f;
	float b = float(color >> 8 & 0xff) / 255.f;
	return Vec3(r, g, b);
}

Vec3 Texture::Sample2D(const Vec2& texCoord, const Vec2& ddxy)
{
	// Repeat
	float texx = (texCoord.x - std::floor(texCoord.x)) * (width - 2);
	float texy = (texCoord.y - std::floor(texCoord.y)) * (height - 2);

	float fracx = texx - floor(texx), fracy = texy - floor(texy);
	int x = int(texx), y = int(texy);

	int level = log(sqrt(std::max(ddxy.x, ddxy.y)) * width) / log(2);
	if (level < 0) level = 0;
	if (level > 11) level = mipmapLevel - 1;
	Vec3 c0 = SamplePixel(x, y, level);
	Vec3 c1 = SamplePixel(x + 1, y, level);
	Vec3 c2 = SamplePixel(x, y + 1, level);
	Vec3 c3 = SamplePixel(x + 1, y + 1, level);

	return Util::Lerp(Util::Lerp(c0, c1, fracx), Util::Lerp(c2, c3, fracx), fracy);
}