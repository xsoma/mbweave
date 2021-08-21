#pragma once
#include <string>
#include <time.h>
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <map>
#include <vector>
#include "ImGui\imgui.h"
#include "ImGui\imgui_impl_dx9.h"
#include "ImGui\imgui_impl_win32.h"
#include "imgui\imgui_freetype.h"
#include <vector>
#include "Vector.h"
#include "colors.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

struct Vertex_t;
class Vector;
class Vector2D;

struct vertex
{
	float x, y, z, rhw;
	uint32_t color;
};

namespace fonts
{
	extern ImFont* esp_name;
	extern ImFont* esp_info;
	extern ImFont* esp_logs;
	extern ImFont* lby_indicator;
	extern ImFont* menu_main;
	extern ImFont* menu_desc;
	extern ImFont* very_small;
	extern ImFont* esp_icons;
	extern ImFont* esp_icons_big;
	extern std::map<uint32_t, ImFont*> js_fonts;
}

namespace render {
	enum e_textflags {
		none = 0,
		outline = 1 << 0,
		centered_x = 1 << 1,
		centered_y = 1 << 2,
		dropshadow = 1 << 3,
	};
}

class ImGuiRendering {
public:
	void __stdcall CreateObjects(IDirect3DDevice9* pDevice);
	void __stdcall InvalidateObjects();
	void __stdcall PreRender(IDirect3DDevice9* device);
	void __stdcall EndPresent(IDirect3DDevice9* device);
	void __stdcall SetupPresent(IDirect3DDevice9* device);

	void DrawEspBox(Vector leftUpCorn, Vector rightDownCorn, color_t clr, float width);
	void DrawLine(float x1, float y1, float x2, float y2, color_t clr, float thickness = 1.f);
	void DrawLineGradient(float x1, float y1, float x2, float y2, color_t clr1, color_t cl2, float thickness = 1.f);
	void Rect(float x, float y, float w, float h, color_t clr, float rounding = 0.f);
	void FilledRect(float x, float y, float w, float h, color_t clr, float rounding = 0.f, ImDrawCornerFlags rounding_corners = 15);
	void FilledRectGradient(float x, float y, float w, float h, color_t col_upr_left, color_t col_upr_right, color_t col_bot_right, color_t col_bot_left);
	void Triangle(float x1, float y1, float x2, float y2, float x3, float y3, color_t clr, float thickness = 1.f);
	void TriangleFilled(float x1, float y1, float x2, float y2, float x3, float y3, color_t clr);
	void Circle(float x1, float y1, float radius, color_t col, int segments);
	void CircleFilled(float x1, float y1, float radius, color_t col, int segments);
	void DrawString(float x, float y, color_t color, int flags, ImFont* font, const char* message, ...);
	void Arc(float x, float y, float radius, float min_angle, float max_angle, color_t col, float thickness = 1.f);
	IDirect3DDevice9* GetDevice() {
		return m_pDevice;
	}
	ImDrawList* _drawList;
private:

	IDirect3DDevice9* m_pDevice;
	
	IDirect3DTexture9* _texture;
	ImFontAtlas _fonts;
}; extern ImGuiRendering* g_Render;