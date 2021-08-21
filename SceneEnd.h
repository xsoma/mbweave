#include "Hooks.h"
#include "Glow.h"

void __fastcall Hooked_SceneEnd(void* thisptr, void* edx)
{
	static auto SceneEnd = g_pRenderViewHook->GetOriginal<SceneEndFn>(g_HookIndices[fnva1(hs::Hooked_SceneEnd.s().c_str())]);
	SceneEnd(thisptr, edx);
}