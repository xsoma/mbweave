#include "render.h"
#include "Hooks.h"

namespace fonts
{
	ImFont* esp_name = nullptr;
	ImFont* esp_info = nullptr;
	ImFont* esp_logs = nullptr;
	ImFont* lby_indicator = nullptr;
	ImFont* menu_main = nullptr;
	ImFont* menu_desc = nullptr;
	ImFont* very_small = nullptr;
	ImFont* esp_icons = nullptr;
	ImFont* esp_icons_big = nullptr;
	std::map<uint32_t, ImFont*> js_fonts;
}

ImGuiRendering* g_Render = new ImGuiRendering();

void __stdcall ImGuiRendering::CreateObjects(IDirect3DDevice9* pDevice)
{
	//if (csgo->Init.Window)
	ImGui_ImplDX9_CreateDeviceObjects();
	_drawList = ImGui::GetBackgroundDrawList();
}

void __stdcall ImGuiRendering::InvalidateObjects()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	_drawList = nullptr;
}

void __stdcall ImGuiRendering::SetupPresent(IDirect3DDevice9* device)
{
	_drawList = ImGui::GetBackgroundDrawList();
}

void __stdcall ImGuiRendering::EndPresent(IDirect3DDevice9* device)
{
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void __stdcall ImGuiRendering::PreRender(IDirect3DDevice9* device)
{
	m_pDevice = device;
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiRendering::DrawString(float x, float y, color_t color, int flags, ImFont* font, const char* message, ...)
{
	char output[4096] = {};
	va_list args;
	va_start(args, message);
	vsprintf(output, message, args);
	va_end(args);

	_drawList->PushTextureID(font->ContainerAtlas->TexID);
	ImGui::PushFont(font);
	auto coord = ImVec2(x, y);
	auto size = ImGui::CalcTextSize(output);
	auto coord_out = ImVec2{ coord.x + 1.f, coord.y + 1.f };
	color_t outline_clr = color_t(0, 0, 0, color.get_alpha() * 0.3f);

	int width = 0, height = 0;
	
	if (!(flags & render::centered_x))
		size.x = 0;
	if (!(flags & render::centered_y))
		size.y = 0;

	ImVec2 pos = ImVec2(coord.x - (size.x * .5), coord.y - (size.y * .5));

	if (flags & render::outline)
	{
		_drawList->AddText(ImVec2(pos.x + 1, pos.y - 1), outline_clr.u32(), output);
		_drawList->AddText(ImVec2(pos.x - 1, pos.y + 1), outline_clr.u32(), output);
		_drawList->AddText(ImVec2(pos.x - 1, pos.y - 1), outline_clr.u32(), output);
		_drawList->AddText(ImVec2(pos.x + 1, pos.y + 1), outline_clr.u32(), output);

		_drawList->AddText(ImVec2(pos.x, pos.y + 1), outline_clr.u32(), output);
		_drawList->AddText(ImVec2(pos.x, pos.y - 1), outline_clr.u32(), output);
		_drawList->AddText(ImVec2(pos.x + 1, pos.y), outline_clr.u32(), output);
		_drawList->AddText(ImVec2(pos.x - 1, pos.y), outline_clr.u32(), output);
	}

	if (flags & render::dropshadow)
	{
		_drawList->AddText(ImVec2(pos.x + 1, pos.y + 1), outline_clr.u32(), output);
	}

	_drawList->AddText(pos, color.u32(), output);
	ImGui::PopFont();
}

void ImGuiRendering::DrawEspBox(Vector leftUpCorn, Vector rightDownCorn, color_t clr, float width)
{
	ImVec2 min = ImVec2(leftUpCorn.x, leftUpCorn.y);
	ImVec2 max = ImVec2(rightDownCorn.x, rightDownCorn.y);
	_drawList->AddRect(min, max, clr.u32(), 0.0F, -1, width);
}

void ImGuiRendering::DrawLine(float x1, float y1, float x2, float y2, color_t clr, float thickness)
{
	_drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), clr.u32(), thickness);
}

void ImGuiRendering::DrawLineGradient(float x1, float y1, float x2, float y2, color_t clr1, color_t cl2, float thickness)
{
	_drawList->AddRectFilledMultiColor(ImVec2(x1, y2), ImVec2(x2 + thickness, y2 + thickness),
		clr1.u32(), cl2.u32(), cl2.u32(), clr1.u32());
}

void ImGuiRendering::Rect(float x, float y, float w, float h, color_t clr, float rounding)
{
	_drawList->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), clr.u32(), rounding);
}

void ImGuiRendering::FilledRect(float x, float y, float w, float h, color_t clr, float rounding, ImDrawCornerFlags rounding_corners)
{
	_drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), clr.u32(), rounding, rounding_corners);
}

void ImGuiRendering::FilledRectGradient(float x, float y, float w, float h, color_t col_upr_left,
	color_t col_upr_right, color_t col_bot_right, color_t col_bot_left)
{
	_drawList->AddRectFilledMultiColor(ImVec2(x, y), ImVec2(x + w, y + h),
		col_upr_left.u32(), col_upr_right.u32(), col_bot_right.u32(), col_bot_left.u32());
}

void ImGuiRendering::Arc(float x, float y, float radius, float min_angle, float max_angle, color_t col, float thickness) {
	_drawList->PathArcTo(ImVec2(x, y), radius, DEG2RAD(min_angle), DEG2RAD(max_angle), 32);
	_drawList->PathStroke(col.u32(), true, thickness);
}

void ImGuiRendering::Triangle(float x1, float y1, float x2, float y2, float x3, float y3, color_t clr, float thickness)
{
	_drawList->AddTriangle(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), clr.u32(), thickness);
}

void ImGuiRendering::TriangleFilled(float x1, float y1, float x2, float y2, float x3, float y3, color_t clr)
{
	_drawList->AddTriangleFilled(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), clr.u32());
}

void ImGuiRendering::Circle(float x1, float y1, float radius, color_t col, int segments)
{
	_drawList->AddCircle(ImVec2(x1, y1), radius, col.u32(), segments);
}

void ImGuiRendering::CircleFilled(float x1, float y1, float radius, color_t col, int segments)
{
	_drawList->AddCircleFilled(ImVec2(x1, y1), radius, col.u32(), segments);
}