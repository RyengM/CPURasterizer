#include <WinEditor/WinBuffer.h>

WinBuffer::WinBuffer()
{
	rasterizer = std::make_unique<Rasterizer>();
	rasterizer->Init();

	int w = rasterizer->frameBuffer->GetWidth(), h = rasterizer->frameBuffer->GetHeight();

	buffer = new unsigned char[w * h * 3];

	ZeroMemory(&bitInfo, sizeof(BITMAPINFO));
	bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitInfo.bmiHeader.biWidth = w;
	bitInfo.bmiHeader.biHeight = h;
	bitInfo.bmiHeader.biPlanes = 1;
	bitInfo.bmiHeader.biBitCount = 24;
	bitInfo.bmiHeader.biCompression = BI_RGB;
}

void WinBuffer::Draw(HWND hWnd, HDC hdc)
{
	rasterizer->Update();

	int w = rasterizer->frameBuffer->GetWidth(), h = rasterizer->frameBuffer->GetHeight();

	unsigned char* rBuffer = rasterizer->frameBuffer->GetBuffer();
	for (int i = 0; i < h; ++i)
		for (int j = 0; j < w; ++j)
		{
			int idr = (i * w + j) * 4;
			int id = (i * w + j) * 3;
			buffer[id + 2] = rBuffer[idr];
			buffer[id + 1] = rBuffer[idr + 1];
			buffer[id] = rBuffer[idr + 2];
		}

	RECT rect;
	GetClientRect(hWnd, &rect);
	SetStretchBltMode(hdc, HALFTONE);
	StretchDIBits(hdc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0, 0, w, h,
		buffer, &bitInfo, DIB_RGB_COLORS, SRCCOPY);
}