#include "Features.h"

void __stdcall Hooked_PaintTraverse(unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{
	static auto PaintTraverse = g_pPanelHook->GetOriginal< PaintTraverseFn >(g_HookIndices[fnva1(hs::Hooked_PaintTraverse.s().c_str())]);

	if (csgo->DoUnload) {
		interfaces.v_panel->SetMouseInputEnabled(vguiPanel, 0);
		return PaintTraverse(interfaces.v_panel, vguiPanel, forceRepaint, allowForce);
	}

	std::string panel_name = interfaces.v_panel->GetName(vguiPanel);

	static std::string HudZoom = str("HudZoom");
	static std::string FocusOverlayPanel = str("FocusOverlayPanel");

	static bool init = false;

	static int ScreenSize2W = -1;
	static int ScreenSize2H = -1;

	if (!init
		|| (ScreenSize2W != csgo->w
			|| ScreenSize2H != csgo->h))
	{
		init = true;
		interfaces.engine->GetScreenSize(ScreenSize2W, ScreenSize2H);
		csgo->w = ScreenSize2W;
		csgo->h = ScreenSize2H;
	}

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			csgo->viewMatrix[i][j] = interfaces.engine->WorldToScreenMatrix()[i][j];

	if (panel_name == HudZoom && vars.visuals.remove & 8)
		return;

	PaintTraverse(interfaces.v_panel, vguiPanel, forceRepaint, allowForce);

	if (panel_name == FocusOverlayPanel) {

		if (interfaces.engine->IsInGame() && interfaces.engine->IsConnected() && csgo->local)
		{
			csgo->is_local_alive = csgo->local->isAlive();

			g_GrenadePrediction->ParseInfo();
			g_Visuals->RecordInfo();
		}
		else
			csgo->is_local_alive = false;

		interfaces.v_panel->SetMouseInputEnabled(vguiPanel, vars.menu.open);
	}

	if (interfaces.engine->IsInGame() && interfaces.engine->IsConnected() && csgo->local)
	{
		static auto viewmodel_offset_x = interfaces.cvars->FindVar(hs::viewmodel_offset_x.s().c_str());
		static auto viewmodel_offset_y = interfaces.cvars->FindVar(hs::viewmodel_offset_y.s().c_str());
		static auto viewmodel_offset_z = interfaces.cvars->FindVar(hs::viewmodel_offset_z.s().c_str());
		static auto weapon_debug_spread_show = interfaces.cvars->FindVar(hs::weapon_debug_spread_show.s().c_str());

		static float old_value_x = viewmodel_offset_x->GetFloat();
		static float old_value_y = viewmodel_offset_y->GetFloat();
		static float old_value_z = viewmodel_offset_z->GetFloat();

		viewmodel_offset_x->m_fnChangeCallbacks.m_Size = 0;
		viewmodel_offset_y->m_fnChangeCallbacks.m_Size = 0;
		viewmodel_offset_z->m_fnChangeCallbacks.m_Size = 0;
		weapon_debug_spread_show->m_fnChangeCallbacks.m_Size = 0;

		viewmodel_offset_x->SetValue(vars.misc.viewmodel_x);
		viewmodel_offset_y->SetValue(vars.misc.viewmodel_y);
		viewmodel_offset_z->SetValue(vars.misc.viewmodel_z);

		if (csgo->local->isAlive())
			weapon_debug_spread_show->SetValue(vars.visuals.force_crosshair && !csgo->local->IsScoped() ? 3 : 0);
	}
}

float __fastcall Hooked_GetScreenAspectRatio(void* pEcx, void* pEdx, int32_t iWidth, int32_t iHeight)
{
	auto GetScreenAspectRatio = g_pEngineHook->GetOriginal< AspectRatioFn >(g_HookIndices[fnva1(hs::Hooked_GetScreenAspectRatio.s().c_str())]);

	if (vars.visuals.aspect_ratio == 0.f)
		return GetScreenAspectRatio(pEcx, iWidth, iHeight);
	else
		return vars.visuals.aspect_ratio / 100.f;
}