#pragma once
#include <d2d1.h>
#include <map>
#include <string>
#include <tchar.h>
#include <chrono>
#include <array>
#include <stdexcept>

#include "GJScene.h"

class GJRenderer {
public:
	void init(HWND _hWnd, GJScene* _scene) {
		hWnd = _hWnd;
		scene = _scene;

		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
		checkFailed(hr, hWnd);

		// Get the size of the client area
		RECT rc;
		GetClientRect(hWnd, &rc);

		// Create a render target
		hr = pFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(
				hWnd,
				D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)
			),
			&pRenderTarget
		);

		checkFailed(hr, hWnd);

		// Create a solid color brush
		hr = pRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF(0.1, 0.1, 0.1)),
			&brushes["black"]
		);
		checkFailed(hr, hWnd);

		hr = pRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF(0.7, 1.0, 0.7)),
			&brushes["green"]
		);
		checkFailed(hr, hWnd);

		hr = pRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF(1.0, 1.0, 0.7)),
			&brushes["amber"]
		);
		checkFailed(hr, hWnd);

		hr = pRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF(0.7, 0.7, 1.0)),
			&brushes["blue"]
		);
		checkFailed(hr, hWnd);

		// Setup transformations
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		float scaleF = static_cast<float>(screenHeight) / 360.f;
		D2D1::Matrix3x2F scale = D2D1::Matrix3x2F::Scale(
			scaleF, scaleF,
			D2D1::Point2F(0.f, 0.f)
		);

		float offsetF = (screenWidth - scaleF * 360.f) / 2.f;
		D2D1::Matrix3x2F translation = D2D1::Matrix3x2F::Translation(
			offsetF, // X offset
			0.f  // Y offset
		);

		D2D1::Matrix3x2F transform = scale * translation;

		pRenderTarget->SetTransform(transform);

		// Setup Draw Call Table
		drawCallTable[static_cast<size_t>(State::INGAME)] = &drawINGAME;
		drawCallTable[static_cast<size_t>(State::LOSS)] = &drawLOSS;
		drawCallTable[static_cast<size_t>(State::WIN)] = &drawWIN;
		drawCallTable[static_cast<size_t>(State::MAINMENU)] = &drawMAINMENU;
		drawCallTable[static_cast<size_t>(State::PAUSED)] = &drawPAUSED;
		if (static_cast<uint32_t>(State::size) != 5) {
			throw std::runtime_error("update state handling in renderer\n");
		}

	}

	~GJRenderer() {
		releaseResources();
	}

	void releaseResources() {
		for (auto& pair : brushes) {
			auto& pBrush = std::get<ID2D1SolidColorBrush*>(pair);
			pBrush->Release();
			pBrush = nullptr;
		}

		if (pRenderTarget)
		{
			pRenderTarget->Release();
			pRenderTarget = nullptr;
		}

		if (pFactory)
		{
			pFactory->Release();
			pFactory = nullptr;
		}

	}

	void wmResize(HWND hwnd) {
		if (pRenderTarget)
		{
			RECT rc;
			GetClientRect(hwnd, &rc);
			pRenderTarget->Resize(D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top));
		}
	}

	void draw(std::chrono::duration<double, std::milli> delta) {
		pRenderTarget->BeginDraw();

		pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

		// 
		(this->*drawCallTable[static_cast<size_t>(scene->state)])();

		HRESULT hr = pRenderTarget->EndDraw();

		if (hr == D2DERR_RECREATE_TARGET)
		{
			releaseResources();
			init(hWnd, scene);
		}

	}

	void drawINGAME() {
		drawScene();
		drawUI();
	}
	void drawMAINMENU() {
		drawMenu("Main Menu");
	}
	void drawLOSS() {
		drawMenu("Loss");
	}
	void drawWIN() {
		drawMenu("Victory");
	}
	void drawPAUSED() {
		drawMenu("Paused");
	}

	void drawScene() {
		D2D1_RECT_F unitSquare = D2D1::RectF(
			20.f, 20.f,
			100.f, 100.f
		);
		pRenderTarget->DrawRectangle(unitSquare, brushes["green"], 3.0f);

	}

	void drawUI() {
		D2D1_RECT_F unitSquare = D2D1::RectF(
			0.f, 0.f,
			360.f, 360.f
		);
		pRenderTarget->DrawRectangle(unitSquare, brushes["amber"], 2.0f);

	}

	void drawMenu(const std::string& text) {
		// todo
	}

private:
	void checkFailed(HRESULT hr, HWND hwnd) {
		if (FAILED(hr))
		{
			MessageBox(NULL, _T("Direct2D Initialization Failed!"), _T("Error"), MB_OK);
			DestroyWindow(hwnd);
			CoUninitialize();
			exit(-1);
		}

	}

private:
	HWND hWnd;
	GJScene* scene = nullptr;
	ID2D1HwndRenderTarget* pRenderTarget = nullptr;
	ID2D1Factory* pFactory = nullptr;
	std::map<std::string, ID2D1SolidColorBrush*> brushes = {
		{"black", nullptr },
		{"green", nullptr },
		{"amber", nullptr },
		{"blue", nullptr } };
	using DrawFunction = void(GJRenderer::*)();
	std::array<DrawFunction, static_cast<size_t>(State::size)> drawCallTable;

};
