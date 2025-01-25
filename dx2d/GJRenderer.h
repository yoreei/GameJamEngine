#pragma once
#include <map>
#include <string>
#include <tchar.h>
#include <chrono>
#include <array>
#include <stdexcept>

#include <windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include "GJScene.h"

enum class TextFormat {
	HEADING = 0,
	NORMAL,
	SMALL,
	size
};

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
		hr = pFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(
				hWnd,
				D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)
			),
			&pRenderTarget
		);

		checkFailed(hr, hWnd);

		// remove antialiasing for retro look
		pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

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
			L"Press Start 2P",                // Font family
			nullptr,                 // Font collection
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			30.0f,                   // Font size
			L"",                     // Locale
			&textFormats[static_cast<size_t>(TextFormat::HEADING)]
		);
		hr = textFormats[static_cast<size_t>(TextFormat::HEADING)]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		hr = textFormats[static_cast<size_t>(TextFormat::HEADING)]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

		hr = pDWriteFactory->CreateTextFormat(
			L"Press Start 2P",                // Font family
			nullptr,                 // Font collection
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			16.0f,                   // Font size
			L"",                     // Locale
			&textFormats[static_cast<size_t>(TextFormat::NORMAL)]
		);
		hr = textFormats[static_cast<size_t>(TextFormat::NORMAL)]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		hr = textFormats[static_cast<size_t>(TextFormat::NORMAL)]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

		hr = pDWriteFactory->CreateTextFormat(
			L"Press Start 2P",                // Font family
			nullptr,                 // Font collection
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			12.0f,                   // Font size
			L"",                     // Locale
			&textFormats[static_cast<size_t>(TextFormat::SMALL)]
		);
		hr = textFormats[static_cast<size_t>(TextFormat::SMALL)]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		hr = textFormats[static_cast<size_t>(TextFormat::SMALL)]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);


		if (static_cast<size_t>(TextFormat::size) != 3) {
			MessageBox(NULL, L"update textformat", L"Error", MB_OK);
			exit(-1);
		}

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

		if (pLowResRenderTarget) {
			pLowResRenderTarget->Release();

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

		drawBorder();
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
		drawEnd(L"Loss");
	}
	void drawWIN() {
		drawEnd(L"Victory");
	}
	void drawPAUSED() {
		drawPaused();
	}

	void drawScene() {

		// Create a compatible render target for low-res drawing
		HRESULT hr = pRenderTarget->CreateCompatibleRenderTarget(
			D2D1::SizeF(360.0f, 360.0f),
			&pLowResRenderTarget
		);
		if (FAILED(hr))
		{
			throw std::runtime_error("");
		}

		// Disable anti-aliasing for pixelated look
		pLowResRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		pLowResRenderTarget->BeginDraw();

		for (const Entity& e : scene->entities) {
			e.position;
			D2D1_RECT_F unitSquare = D2D1::RectF(
				e.position.e[0], e.position.e[1],
				e.position.e[0] + e.size, e.position.e[1] + e.size
			);
			pLowResRenderTarget->DrawRectangle(unitSquare, brushes["blue"], 2.f);
		}
		for (const Entity& o : scene->obstacles) {
			o.position;
			D2D1_ELLIPSE ellipse = D2D1::Ellipse(
				D2D1::Point2F(o.position.x(), o.position.y()), // Center point (x, y)
				o.size,                         // Radius X
				o.size                          // Radius Y
			);
			pLowResRenderTarget->FillEllipse(ellipse, brushes["green"]);

		}
		pLowResRenderTarget->EndDraw();

		ID2D1Bitmap* pLowResBitmap = nullptr;
		hr = pLowResRenderTarget->GetBitmap(&pLowResBitmap);

		// todo: draw pGridBitmap onto pRenderTarget
		D2D1_RECT_F destRect = D2D1::RectF(
			0.0f,                         // Left
			0.0f,                         // Top
			360.f,  // Right
			360.f  // Bottom
		);
		pRenderTarget->DrawBitmap(
			pLowResBitmap,                                    // The bitmap to draw
			&destRect,                                      // Destination rectangle
			1.0f,                                           // Opacity (1.0f = fully opaque)
			D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, // Nearest-neighbor for pixelated scaling
			NULL                                            // Source rectangle (NULL to use the entire bitmap)
		);
		pLowResBitmap->Release();




	}

	void drawUI() {

	}

	void drawBorder() {
		D2D1_RECT_F unitSquare = D2D1::RectF(
			0.f, 0.f,
			360.f, 360.f
		);
		pRenderTarget->DrawRectangle(unitSquare, brushes["amber"], 2.f);

	}

	void drawPaused() {
		pRenderTarget->DrawText(
			L"Paused",    // Text to render
			wcslen(L"Paused"),
			textFormats[static_cast<size_t>(TextFormat::HEADING)],            // Text format
			D2D1::RectF(0, 40, 360, 180), // Layout rectangle
			brushes["blue"]
		);

		std::wstring t = L"[ESC] Resume\n[R] Reload\n[BSPACE] Quit";
		pRenderTarget->DrawText(
			t.c_str(),    // Text to render
			wcslen(t.c_str()),
			textFormats[static_cast<size_t>(TextFormat::NORMAL)],            // Text format
			D2D1::RectF(0, 180, 320, 210), // Layout rectangle
			brushes["blue"]
		);
	}

	void drawEnd(const std::wstring& text) {
		pRenderTarget->DrawText(
			text.c_str(),    // Text to render
			wcslen(text.c_str()),
			textFormats[static_cast<size_t>(TextFormat::HEADING)],            // Text format
			D2D1::RectF(0, 40, 360, 180), // Layout rectangle
			brushes["blue"]
		);

		pRenderTarget->DrawText(
			L"[R] Reload\n[BSPACE] Quit",    // Text to render
			wcslen(L"[R] Reload\n[BSPACE] Quit"),
			textFormats[static_cast<size_t>(TextFormat::NORMAL)],            // Text format
			D2D1::RectF(0, 180, 320, 210), // Layout rectangle
			brushes["blue"]
		);

	}
	void drawMenu(const std::string& text) {
		// todo
		pRenderTarget->DrawText(
			L"Electric\nBubble\nBath!",    // Text to render
			wcslen(L"Electric\nBubble\nBath!"),
			textFormats[static_cast<size_t>(TextFormat::HEADING)],            // Text format
			D2D1::RectF(0, 40, 360, 180), // Layout rectangle
			brushes["blue"]
		);

		pRenderTarget->DrawText(
			L"[ENTER] Game\n[BSPACE] Quit",    // Text to render
			wcslen(L"[ENTER] Game\nF[BSPACE] Quit"),
			textFormats[static_cast<size_t>(TextFormat::NORMAL)],            // Text format
			D2D1::RectF(0, 180, 320, 210), // Layout rectangle
			brushes["blue"]
		);

		//pRenderTarget->DrawText(
		//	L"[F10] Terminate Program",    // Text to render
		//	wcslen(L"[F10] Terminate Program"),
		//	textFormats[static_cast<size_t>(TextFormat::NORMAL)],            // Text format
		//	D2D1::RectF(0, 210, 320, 210), // Layout rectangle
		//	brushes["blue"]
		//);

		//D2D1_RECT_F unitSquare = D2D1::RectF(
		//	20.f, 20.f,
		//	100.f, 100.f
		//);
		//pRenderTarget->DrawRectangle(unitSquare, brushes["blue"], 3.f);

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
	ID2D1BitmapRenderTarget* pLowResRenderTarget = nullptr;
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

