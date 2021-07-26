#include <WinEditor/WinBuffer.h>
#include <windows.h>

using namespace LearnTask;

WinBuffer renderer;

static HDC hdcBackBuffer;
static HBITMAP hBitmap;
static HBITMAP hOldBitmap;
static int wClient, hClinet;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	// Register class
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = _T("SoftRenderer");
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE((WORD)IDI_APPLICATION));
	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Win32 Guided Tour"),
			NULL);

		return 1;
	}

	// Create window
	HWND hWnd = CreateWindow(
		wcex.lpszClassName,
		wcex.lpszClassName,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT,
		renderer.rasterizer->frameBuffer->GetWidth(), renderer.rasterizer->frameBuffer->GetHeight(),
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if (!hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("Win32 Guided Tour"),
			NULL);

		return 1;
	}
	
	// Show window
	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	// Start message loop
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			HDC hdc = GetDC(hWnd);
			BitBlt(hdcBackBuffer, 0, 0, wClient, hClinet, NULL, NULL, NULL, BLACKNESS);
			renderer.Draw(hWnd, hdcBackBuffer);
			BitBlt(hdc, 0, 0, wClient, hClinet, hdcBackBuffer, 0, 0, SRCCOPY);
			ReleaseDC(hWnd, hdc);
		}
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		RECT rect;
		GetClientRect(hWnd, &rect);
		wClient = rect.right;
		hClinet = rect.bottom;
		hdcBackBuffer = CreateCompatibleDC(NULL);
		hdc = GetDC(hWnd);
		hBitmap = CreateCompatibleBitmap(hdc, wClient, hClinet);
		// Install bitmap to dc
		hOldBitmap = (HBITMAP)SelectObject(hdcBackBuffer, hBitmap);
		ReleaseDC(hWnd, hdc);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		SelectObject(hdcBackBuffer, hOldBitmap);
		DeleteDC(hdcBackBuffer);
		DeleteObject(hBitmap);
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		renderer.rasterizer->deviceStatus->leftMouseActive = true;
		break;
	case WM_LBUTTONUP:
		renderer.rasterizer->deviceStatus->leftMouseActive = false;
		break;
	case WM_RBUTTONDOWN:
		renderer.rasterizer->deviceStatus->rightMouseActive = true;
		break;
	case WM_RBUTTONUP:
		renderer.rasterizer->deviceStatus->rightMouseActive = false;
		break;
	case WM_MOUSEMOVE:
		renderer.rasterizer->deviceStatus->ProcessMouseMovement(LOWORD(lParam), HIWORD(lParam));
		renderer.rasterizer->camera->ProcessMouseMovement();
		renderer.rasterizer->dirLight->ProcessMouseMovement();
		break;
	case WM_SYSKEYDOWN:
		switch (wParam)
		{
		// Alt
		case 18:
			renderer.rasterizer->deviceStatus->leftAltActive = true;
			break;
		default:
			break;
		}
		break;
	case WM_SYSKEYUP:
		switch (wParam)
		{
		// Alt
		case 18:
			renderer.rasterizer->deviceStatus->leftAltActive = false;
			break;
		default:
			break;
		}
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		// Shift
		case 16:
			renderer.rasterizer->deviceStatus->leftShiftActive = true;
			break;
		// TAB
		case 9:
			renderer.rasterizer->SwitchModel();
			renderer.rasterizer->bModelSwitching = true;
			break;
		// 1
		case 49:
			renderer.rasterizer->renderMode = ERenderMode::WireFrame;
			break;
		// 2
		case 50:
			renderer.rasterizer->renderMode = ERenderMode::Render;
			break;
		// P
		case 80:
			renderer.rasterizer->shaders["phong"]->SwitchShadow();
			renderer.rasterizer->shaders["phong"]->bShadowSwitch = true;
			break;
		// Q
		case 81:
			renderer.rasterizer->SwitchCullMode();
			renderer.rasterizer->bFaceCullSwitching = true;
			break;
		default:
			break;
		}
		break;
	case WM_KEYUP:
		switch (wParam)
		{
		// Shift
		case 16:
			renderer.rasterizer->deviceStatus->leftShiftActive = false;
			break;
		// TAB
		case 9:
			renderer.rasterizer->bModelSwitching = false;
			break;
		// P
		case 80:
			renderer.rasterizer->shaders["phong"]->bShadowSwitch = false;
			break;
		// Q
		case 81:
			renderer.rasterizer->bFaceCullSwitching = false;
			break;
		default:
			break;
		}
		break;
	case WM_MOUSEWHEEL:
		renderer.rasterizer->camera->ProcessMouseScroll(short(HIWORD(wParam)) * 0.01f);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}