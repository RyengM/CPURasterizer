#pragma once

#include <Renderer/Rasterizer.h>
#include <WinEditor/stdafx.h>
#include <memory>

using namespace LearnTask;

class WinBuffer
{
public:
	WinBuffer();

	void Draw(HWND hWnd, HDC hdc);

public:
	std::unique_ptr<Rasterizer> rasterizer;

	unsigned char* buffer;
	BITMAPINFO bitInfo;
};