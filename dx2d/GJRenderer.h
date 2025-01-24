#pragma once
#include <map>
#include <string>
#include <tchar.h>
#include <chrono>
#include <array>
#include <stdexcept>

#include <d2d1.h>
#include <dwrite.h>

#include "GJScene.h"

class GJRenderer {
public:
	void init(HWND _hWnd, const GJScene* _scene) {
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
			D2D1::ColorF(D2D1::ColorF(0.1f, 0.1f, 0.1f)),
			&brushes["black"]
		);
		checkFailed(hr, hWnd);

		hr = pRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF(0.7f, 1.f, 0.7f)),
			&brushes["green"]
		);
		checkFailed(hr, hWnd);

		hr = pRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF(1.f, 1.f, 0.7f)),
			&brushes["amber"]
		);
		checkFailed(hr, hWnd);

		hr = pRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF(0.7f, 0.7f, 1.f)),
			&brushes["blue"]
		);
		checkFailed(hr, hWnd);

		IDWriteFactory* pDWriteFactory = nullptr;
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));

		hr = pDWriteFactory->CreateTextFormat(
			L"Arial",                // Font family
			nullptr,                 // Font collection
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			42.0f,                   // Font size
			L"",                     // Locale
			&textFormats[static_cast<size_t>(TextFormat::HEADING)]
		);

		hr = pDWriteFactory->CreateTextFormat(
			L"Arial",                // Font family
			nullptr,                 // Font collection
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			24.0f,                   // Font size
			L"",                     // Locale
			&textFormats[static_cast<size_t>(TextFormat::NORMAL)]
		);

		hr = pDWriteFactory->CreateTextFormat(
			L"Arial",                // Font family
			nullptr,                 // Font collection
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			16.0f,                   // Font size
			L"",                     // Locale
			&textFormats[static_cast<size_t>(TextFormat::SMALL)]
		);
		if (static_cast<size_t>(TextFormat::size) != 3) {
			MessageBox(NULL, L"update textformat", L"Error", MB_OK);
			exit(-1);
		}

		// Set Text Alignment
		hr = pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		hr = pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

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
		drawCallTable[static_cast<size_t>(State::INGAME)] = &GJRenderer::drawINGAME;
		drawCallTable[static_cast<size_t>(State::LOSS)] = &GJRenderer::drawLOSS;
		drawCallTable[static_cast<size_t>(State::WIN)] = &GJRenderer::drawWIN;
		drawCallTable[static_cast<size_t>(State::MAINMENU)] = &GJRenderer::drawMAINMENU;
		drawCallTable[static_cast<size_t>(State::PAUSED)] = &GJRenderer::drawPAUSED;
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
		pRenderTarget->DrawRectangle(unitSquare, brushes["green"], 3.f);

	}

	void drawUI() {
		D2D1_RECT_F unitSquare = D2D1::RectF(
			0.f, 0.f,
			360.f, 360.f
		);
		pRenderTarget->DrawRectangle(unitSquare, brushes["amber"], 2.f);

	}

	void drawMenu(const std::string& text) {
		// todo
		pRenderTarget->DrawText(
			L"Hello, Direct2D!",    // Text to render
			wcslen(L"Hello, Direct2D!"),
			textFormats[static_cast<size_t>(TextFormat::HEADING)],            // Text format
			D2D1::RectF(50, 50, 400, 100), // Layout rectangle
			brushes["blue"]
		);
		D2D1_RECT_F unitSquare = D2D1::RectF(
			20.f, 20.f,
			100.f, 100.f
		);
		pRenderTarget->DrawRectangle(unitSquare, brushes["blue"], 3.f);

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
	const GJScene* scene = nullptr;
	ID2D1HwndRenderTarget* pRenderTarget = nullptr;
	ID2D1Factory* pFactory = nullptr;
	std::map<std::string, ID2D1SolidColorBrush*> brushes = {
		{"black", nullptr },
		{"green", nullptr },
		{"amber", nullptr },
		{"blue", nullptr } };
	std::array<IDWriteTextFormat*, static_cast<size_t>(TextFormat::size)> textFormats;
	using DrawFunction = void(GJRenderer::*)();
	std::array<DrawFunction, static_cast<size_t>(State::size)> drawCallTable;

};

enum class TextFormat {
	HEADING = 0,
	NORMAL,
	SMALL,
	size
};
