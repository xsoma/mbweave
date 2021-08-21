#include "Hooks.h"
#include "Menu.h"
#include "imgui\imgui_impl_dx9.h"
#include "imgui\imgui_impl_win32.h"
#include "imgui\imgui_freetype.h"
#include <d3dx9tex.h>
#include "AntiAims.h"
#include "Eventlog.h"
#include "protect/encoding.hpp"
#include <chrono>
#include "manrope_meduim_ttf.h"
#include "fonts/SMALLEST_PIXEL-7.c"
#include "fonts/OpenSans-SemiBold.c"
#include "fonts/OpenSans-Bold.c"
#include "fonts/undefeated.h"
#include "shonax_moneybot_menu/direct_render_shonax.h"
#include "shonax_moneybot_menu/menu_elements_shonax.h"



void InitRender(IDirect3DDevice9* pDevice)
{
	if (!vars.menu.guiinited)
	{
		ImGui::CreateContext();

		ImGui_ImplWin32_Init(csgo->Init.Window);
		ImGui_ImplDX9_Init(pDevice);

		ImGuiIO& io = ImGui::GetIO();

		ImGuiStyle& style = ImGui::GetStyle();
		style.AntiAliasedFill = false;
		style.AntiAliasedLines = false;

		ImFontConfig cfg;
		cfg.OversampleH = 3;
		cfg.OversampleV = 1;

		fonts::esp_name = io.Fonts->AddFontFromMemoryTTF(OpenSans_Bold, sizeof(OpenSans_Bold), 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

		fonts::esp_logs = io.Fonts->AddFontFromMemoryTTF(OpenSans_Bold, sizeof(OpenSans_Bold), 13.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

		fonts::esp_info = io.Fonts->AddFontFromMemoryTTF(OpenSans_Bold, sizeof(OpenSans_Bold), 12.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

		fonts::esp_icons = io.Fonts->AddFontFromMemoryCompressedTTF(
			undefeated_compressed_data,
			undefeated_compressed_size,
			20.f,
			&cfg,
			io.Fonts->GetGlyphRangesCyrillic());

		fonts::esp_icons_big = io.Fonts->AddFontFromMemoryCompressedTTF(
			undefeated_compressed_data,
			undefeated_compressed_size,
			28.f,
			&cfg,
			io.Fonts->GetGlyphRangesCyrillic());

		int old_fl = cfg.RasterizerFlags;
		cfg.RasterizerFlags = ImGuiFreeType::RasterizerFlags::MonoHinting | ImGuiFreeType::RasterizerFlags::Monochrome;
		fonts::very_small = io.Fonts->AddFontFromMemoryTTF(smallest_pixel_7, sizeof(smallest_pixel_7), 10, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		cfg.RasterizerFlags = old_fl;

		fonts::menu_desc = io.Fonts->AddFontFromMemoryTTF(OpenSans_Bold, sizeof(OpenSans_Bold), 16.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		fonts::menu_main = io.Fonts->AddFontFromMemoryTTF(OpenSans_Bold, sizeof(OpenSans_Bold), 16.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

		fonts::lby_indicator = io.Fonts->AddFontFromFileTTF(str("C:/windows/fonts/verdanab.ttf"), 20.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

		ImGuiFreeType::BuildFontAtlas(io.Fonts, 0x0);

		vars.menu.guiinited = true;
	}
}

LPDIRECT3DTEXTURE9 img = nullptr;
LPD3DXSPRITE sprite = nullptr;

void DrawTaserRange() {
	if (!csgo->should_draw_taser_range || !vars.visuals.taser_range)
		return;
	static float rainbow = 0.f;
	rainbow += 0.001f;
	if (rainbow > 1.f)
		rainbow = 0.f;
	std::pair<Vector, Vector> last = std::pair(Vector(0.f, 0.f, 0.f), Vector(0.f, 0.f, 0.f));
	color_t last_color = color_t(0, 0, 0, 0);
	color_t first_color = color_t(255, 255, 255, 0);

	constexpr size_t size = 106;

	for (size_t i = 0; i < size; i++)
	{
		auto& trace = csgo->taser_ranges[i];
		float r = rainbow + float(i + 1) / size;
		while (r >= 1.f) r -= 1.f;
		while (r < 0.f) r += 1.f;
		color_t clr = color_t::hsb(r, 1.f, 1.f);
		if (i == 0) first_color = clr;
		std::pair<Vector, Vector> buf;
		if (last.first != Vector(0.f, 0.f, 0.f) && last.second != Vector(0.f, 0.f, 0.f)) {
			Vector v1, v2;
			if (Math::WorldToScreen(trace.first, v1) && Math::WorldToScreen(trace.second, v2)) {
				if (i == 105)
					last_color = first_color;

				auto flags_backup = g_Render->_drawList->Flags;
				g_Render->_drawList->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

				if (vars.visuals.taser_range == 1)
					g_Render->_drawList->AddQuadFilledMultiColor(
						ImVec2(last.second.x, last.second.y), ImVec2(v2.x, v2.y), ImVec2(v1.x, v1.y),
						ImVec2(last.first.x, last.first.y),
						last_color.manage_alpha(0).u32(), clr.manage_alpha(0).u32(),
						clr.manage_alpha(255).u32(), last_color.manage_alpha(255).u32());
				else
					g_Render->_drawList->AddQuadFilledMultiColor(
						ImVec2(last.second.x, last.second.y), ImVec2(v2.x, v2.y), ImVec2(v1.x, v1.y),
						ImVec2(last.first.x, last.first.y),
						vars.visuals.taser_range_color.manage_alpha(0).u32(), vars.visuals.taser_range_color.manage_alpha(0).u32(),
						vars.visuals.taser_range_color.u32(), vars.visuals.taser_range_color.u32());

				g_Render->_drawList->Flags = flags_backup;
			}
		}
		last_color = clr;
		Math::WorldToScreen(trace.first, last.first);
		Math::WorldToScreen(trace.second, last.second);
	}
}


void on_d3d(IDirect3DDevice9* device) {
	constexpr float step = 1.0f / 2.f;
	constexpr float fade_step = 1.0f / 7.f;
	static float anim_time{ };
	static bool flip{ };
	static int anim_progress{ };

	static bool sprites_init{ };
	if (!sprites_init) {
		ui::setup_sprites(device);
		sprites_init = true;
	}

	if (vars.menu.open) {
		if (anim_time >= 0.95f) {
			flip = true;
		}
		if (anim_time <= 0.05f) {
			flip = false;
		}

		if (flip)
			anim_time -= fade_step * ui::ui_get_frametime();
		else
			anim_time += fade_step * ui::ui_get_frametime();

		anim_time = std::clamp(anim_time, 0.f, 1.0f);

		ui::set_animtime(anim_time);

		anim_progress += ui::ui_get_frametime() * step * 255;
		anim_progress = std::clamp(anim_progress, 0, 50);
		g_d3d.draw_filled_rect(color_t(0, 0, 0, anim_progress),
			0, 0, g_d3d.m_width, g_d3d.m_height);

		ui::render();
	}
	else {
		anim_progress = 0;
	}
}



HRESULT __stdcall Hooked_EndScene(IDirect3DDevice9* device)
{
	static auto EndScene = g_pDirectHook->GetOriginal<EndSceneFn>(g_HookIndices[fnva1(hs::Hooked_EndScene.s().c_str())]);
	if (csgo->DoUnload) 
		return EndScene(device);
	
	DWORD colorwrite, srgbwrite;
	IDirect3DVertexDeclaration9* vert_dec = nullptr;
	IDirect3DVertexShader9* vert_shader = nullptr;
	DWORD dwOld_D3DRS_COLORWRITEENABLE = NULL;
	device->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
	device->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgbwrite);

	device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
	device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE);

	device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
	//removes the source engine color correction
	device->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

	device->GetVertexDeclaration(&vert_dec);
	device->GetVertexShader(&vert_shader);
	device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
	device->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
	device->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	device->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	device->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
	device->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);


	InitRender(device);
	csgo->render_device = device;
	g_Render->PreRender(device);
	g_Render->SetupPresent(device);
	{
		g_Menu->draw_indicators();

		g_Visuals->Draw();

		g_Hitmarker->Paint();

		DrawTaserRange();

		g_Visuals->DrawLocalVisuals();

		auto flags_backup = g_Render->_drawList->Flags;
		g_Render->_drawList->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;
		run_callback("render");
		g_Render->_drawList->Flags = flags_backup;

		//if (false)
		//if (vars.menu.open)

		//static bool once = false;
		//if (!once) {
		//	Config.ResetToDefault();
		//	vars.menu.open = true;
		//	once = true;
		//}

		g_Menu->render(); //old
		//shonax
		{
			if (g_d3d.run_frame(device)) {
				for (auto& it : ::d3d::sprites)
					it->begin(device);

				g_d3d.begin();
				on_d3d(device);

				std::chrono::high_resolution_clock timer;
				static auto last = timer.now();
				auto now = timer.now();

				std::chrono::duration< double > delta = now - last;
				last = timer.now();

				ui::m_frametime = delta.count();

				//g_Menu->update_binds(); //fix

				
				if (vars.menu.open) 
					ui::ui_draw_cursor();

				for (auto& it : ::d3d::sprites)
					it->end();
				g_d3d.end();
			}
		}
		

		g_Eventlog->Draw();
	}
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	device->SetRenderState(D3DRS_COLORWRITEENABLE, colorwrite);
	device->SetRenderState(D3DRS_SRGBWRITEENABLE, srgbwrite);
	device->SetVertexDeclaration(vert_dec);
	device->SetVertexShader(vert_shader);

	csgo->mtx.lock();
	for (auto& texture : csgo->outdated_textures)
		texture.second->Release();
	csgo->outdated_textures.clear();
	csgo->mtx.unlock();


	return EndScene(device);
}

HRESULT D3DAPI Hooked_Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params)
{
	static auto Reset = g_pDirectHook->GetOriginal<ResetFn>(g_HookIndices[fnva1(hs::Hooked_Reset.s().c_str())]);

	if (csgo->DoUnload)
		return Reset(device, params);

	g_Render->InvalidateObjects();

	for (auto& sprite : ::d3d::sprites)
		sprite->on_reset();
	g_d3d.on_device_lost();

	auto hr = Reset(device, params);
	if (hr >= 0) {
		g_Render->CreateObjects(device);

		g_d3d.on_device_reset();

		for (auto& sprite : ::d3d::sprites)
			sprite->on_reset_end();

	}
	return hr;
}