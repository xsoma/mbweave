#pragma once
#include "Hooks.h"
#include <intrin.h>

// disable model occlusion
#define MAX_COORD_FLOAT ( 16384.0f )
#define MIN_COORD_FLOAT ( -MAX_COORD_FLOAT )

void ClearSmokeEffect()
{
	static auto smokeCount = *reinterpret_cast<uint32_t**>(csgo->Utils.FindPatternIDA(GetModuleHandleA(
		g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::smoke_effect.s().c_str()) + 1);
	*(int*)smokeCount = 0;
}

void __fastcall Hooked_GetColorModulation(void* ecx, void* edx, float* r, float* g, float* b) {
	static auto original = hooker::h.original(&Hooked_GetColorModulation);

	if (csgo->DoUnload)
		return original(ecx,edx, r, g, b);

	original(ecx, edx, r, g, b);

	const auto material = reinterpret_cast<IMaterial*>(ecx);

	if (!material || material->IsErrorMaterial())
		return original(ecx, edx, r, g, b);

	const auto name = material->GetTextureGroupName();

	static std::string World = hs::World.s().c_str();
	static std::string StaticProp = hs::StaticProp.s().c_str();
	static std::string SkyBox = hs::SkyBox.s().c_str();

	auto amount = 100 - vars.visuals.nightmode_amount;

	if (strstr(name, World.c_str())) {
		if (vars.visuals.customize_color) {
			*r *= vars.visuals.nightmode_color[0] / 255.f;
			*g *= vars.visuals.nightmode_color[1] / 255.f;
			*b *= vars.visuals.nightmode_color[2] / 255.f;
		}
		else {
			*r *= amount * 0.01f;
			*g *= amount * 0.01f;
			*b *= amount * 0.01f;
		}
	}
	else if (strstr(name, StaticProp.c_str())) {
		if (vars.visuals.customize_color) {
			*r *= vars.visuals.nightmode_prop_color[0] / 255.f;
			*g *= vars.visuals.nightmode_prop_color[1] / 255.f;
			*b *= vars.visuals.nightmode_prop_color[2] / 255.f;
		}
		else {
			*r *= amount * 0.01f;
			*g *= amount * 0.01f;
			*b *= amount * 0.01f;
		}
	}
	else if (strstr(name, SkyBox.c_str())) {
		if (vars.visuals.customize_color) {
			*r *= vars.visuals.nightmode_skybox_color[0] / 255.f;
			*g *= vars.visuals.nightmode_skybox_color[1] / 255.f;
			*b *= vars.visuals.nightmode_skybox_color[2] / 255.f;
		}
	}
}

bool __fastcall Hooked_IsUsingStaticPropDebugModes(void* ecx, void* edx)
{
	return vars.visuals.nightmode || vars.visuals.prop_alpha_amount > 0;
}

void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix* pCustomBoneToWorld)
{
	static auto DrawModelExecute = g_pModelRenderHook->GetOriginal< DrawModelExecuteFn >(g_HookIndices[fnva1(hs::Hooked_DrawModelExecute.s().c_str())]);

	const auto mN = info.pModel->name;

	if (csgo->DoUnload 
		|| interfaces.studio_render->IsForcedMaterialOverride())
		return DrawModelExecute(interfaces.models.model_render, ctx, state, info, pCustomBoneToWorld);

	if (interfaces.engine->IsInGame() && csgo->local && info.pModel)
	{
		static std::string contats_shadow = str("player/contactshadow");
		static std::string prop = str("props");

		if ((vars.visuals.remove & 128) && strstr(mN, contats_shadow.c_str()) != nullptr)
			return;

		if (strstr(mN, prop.c_str()) != nullptr)
			interfaces.render_view->SetBlend(vars.visuals.customize_color ?
				vars.visuals.nightmode_prop_color.get_alpha() / 255.f
				: (100.f - vars.visuals.prop_alpha_amount) * 0.01f);
		
		static IMaterial* mat1 = interfaces.material_system->FindMaterial(str("particle/vistasmokev1/vistasmokev1_smokegrenade"), str("Other textures"));
		static IMaterial* mat2 = interfaces.material_system->FindMaterial(str("particle/vistasmokev1/vistasmokev1_emods"), str("Other textures"));
		static IMaterial* mat3 = interfaces.material_system->FindMaterial(str("particle/vistasmokev1/vistasmokev1_emods_impactdust"), str("Other textures"));
		static IMaterial* mat4 = interfaces.material_system->FindMaterial(str("particle/vistasmokev1/vistasmokev1_fire"), str("Other textures"));

		mat1->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, vars.visuals.remove & 2);
		mat2->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, vars.visuals.remove & 2);
		mat3->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, vars.visuals.remove & 2);
		mat4->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, vars.visuals.remove & 2);

		if (vars.visuals.remove & 2)
			ClearSmokeEffect();

		if (!g_Chams->ShouldDraw(DrawModelExecute, interfaces.models.model_render, ctx, state, info, pCustomBoneToWorld))
			DrawModelExecute(interfaces.models.model_render, ctx, state, info, pCustomBoneToWorld);
	}
	else
		DrawModelExecute(interfaces.models.model_render, ctx, state, info, pCustomBoneToWorld);

	interfaces.models.model_render->ForcedMaterialOverride(nullptr);
	interfaces.render_view->SetBlend(1.f);
}

int __fastcall Hooked_ListLeavesInBox(void* bsp, void* edx, Vector& mins, Vector& maxs, unsigned short* pList, int listMax) 
{
	static auto ListLeavesInBox = g_pQueryHook->GetOriginal< ListLeavesInBoxFn >(g_HookIndices[fnva1(hs::Hooked_ListLeavesInBox.s().c_str())]);

	// occulusion getting updated on player movement/angle change,
	// in RecomputeRenderableLeaves ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L674 );
	// check for return in CClientLeafSystem::InsertIntoTree

	static auto list_leaves = (void*)(csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		str("56 52 FF 50 18")) + 5);

	if (_ReturnAddress() != list_leaves)
		return ListLeavesInBox(bsp, mins, maxs, pList, listMax);

	// get current renderable info from stack ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1470 )
	auto info = *(RenderableInfo_t**)((uintptr_t)_AddressOfReturnAddress() + 0x14);
	if (!info || !info->m_pRenderable)
		return ListLeavesInBox(bsp, mins, maxs, pList, listMax);

	// check if disabling occulusion for players ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1491 )
	auto base_entity = info->m_pRenderable->GetIClientUnknown()->GetBaseEntity();
	if (!base_entity || !base_entity->IsPlayer() || base_entity->EntIndex() == interfaces.engine->GetLocalPlayer())
		return ListLeavesInBox(bsp, mins, maxs, pList, listMax);

	// fix render order, force translucent group ( https://www.unknowncheats.me/forum/2429206-post15.html )
	// AddRenderablesToRenderLists: https://i.imgur.com/hcg0NB5.png ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L2473 )
	info->m_Flags &= ~0x100;
	info->m_Flags2 |= 0xC0;

	// extend world space bounds to maximum ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L707 )
	static const Vector map_min = Vector(MIN_COORD_FLOAT, MIN_COORD_FLOAT, MIN_COORD_FLOAT);
	static const Vector map_max = Vector(MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT);
	auto count = ListLeavesInBox(bsp, map_min, map_max, pList, listMax);
	return count;
}