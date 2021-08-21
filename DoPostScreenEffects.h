#pragma once
#include "Hooks.h"

bool __fastcall Hooked_DoPostScreenEffects(void* ecx, void* edx, CViewSetup* setup)
{
	static auto DoPostScreenEffects = g_pClientModeHook->GetOriginal< DoPostScreenEffectsFn >(g_HookIndices[fnva1(hs::Hooked_DoPostScreenEffects.s().c_str())]);

	if (csgo->DoUnload)
		return DoPostScreenEffects(ecx, setup);

	if (csgo->local) {
		g_Chams->OnPostScreenEffects();
		g_Glow->Draw();
	}

	return DoPostScreenEffects(ecx, setup);
}