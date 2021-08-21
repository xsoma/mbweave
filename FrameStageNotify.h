#pragma once
#include "Hooks.h"
#include "RageBacktracking.h"
#include "Misc.h"
#include "Resolver.h"
#include "AnimationFix.h"
#include "SetupAnimation.h"
#include <intrin.h>
#include <random>
#include "Ragebot.h"

std::string GetModel(int team) {
	static const std::vector<std::string> models = {
		str("models/player/custom_player/legacy/ctm_fbi_variantb.mdl"),
		str("models/player/custom_player/legacy/ctm_fbi_variantf.mdl"),
		str("models/player/custom_player/legacy/ctm_fbi_variantg.mdl"),
		str("models/player/custom_player/legacy/ctm_fbi_varianth.mdl"),
		str("models/player/custom_player/legacy/ctm_sas_variantf.mdl"),
		str("models/player/custom_player/legacy/ctm_st6_variante.mdl"),
		str("models/player/custom_player/legacy/ctm_st6_variantg.mdl"),
		str("models/player/custom_player/legacy/ctm_st6_varianti.mdl"),
		str("models/player/custom_player/legacy/ctm_st6_variantk.mdl"),
		str("models/player/custom_player/legacy/ctm_st6_variantm.mdl"),
		str("models/player/custom_player/legacy/tm_balkan_variantf.mdl"),
		str("models/player/custom_player/legacy/tm_balkan_variantg.mdl"),
		str("models/player/custom_player/legacy/tm_balkan_varianth.mdl"),
		str("models/player/custom_player/legacy/tm_balkan_varianti.mdl"),
		str("models/player/custom_player/legacy/tm_balkan_variantj.mdl"),
		str("models/player/custom_player/legacy/tm_leet_variantf.mdl"),
		str("models/player/custom_player/legacy/tm_leet_variantg.mdl"),
		str("models/player/custom_player/legacy/tm_leet_varianth.mdl"),
		str("models/player/custom_player/legacy/tm_leet_varianti.mdl"),
		str("models/player/custom_player/legacy/tm_phoenix_variantf.mdl"),
		str("models/player/custom_player/legacy/tm_phoenix_variantg.mdl"),
		str("models/player/custom_player/legacy/tm_phoenix_varianth.mdl"),
		str("models/player/custom_player/legacy/tm_phoenix_varianti.mdl"),
		str("models/player/custom_player/legacy/ctm_st6_variantj.mdl"),
		str("models/player/custom_player/legacy/ctm_st6_variantl.mdl"),
		str("models/player/custom_player/legacy/tm_balkan_variantk.mdl"),
		str("models/player/custom_player/legacy/tm_balkan_variantl.mdl"),
		str("models/player/custom_player/legacy/ctm_swat_variante.mdl"),
		str("models/player/custom_player/legacy/ctm_swat_variantf.mdl"),
		str("models/player/custom_player/legacy/ctm_swat_variantg.mdl"),
		str("models/player/custom_player/legacy/ctm_swat_varianth.mdl"),
		str("models/player/custom_player/legacy/ctm_swat_varianti.mdl"),
		str("models/player/custom_player/legacy/ctm_swat_variantj.mdl"),
		str("models/player/custom_player/legacy/tm_professional_varf.mdl"),
		str("models/player/custom_player/legacy/tm_professional_varf1.mdl"),
		str("models/player/custom_player/legacy/tm_professional_varf2.mdl"),
		str("models/player/custom_player/legacy/tm_professional_varf3.mdl"),
		str("models/player/custom_player/legacy/tm_professional_varf4.mdl"),
		str("models/player/custom_player/legacy/tm_professional_varg.mdl"),
		str("models/player/custom_player/legacy/tm_professional_varh.mdl"),
		str("models/player/custom_player/legacy/tm_professional_vari.mdl"),
		str("models/player/custom_player/legacy/tm_professional_varj.mdl")
	};

	switch (team) {
	case 2: return (size_t)(vars.misc.agents_t - 1) < models.size() ? models[vars.misc.agents_t - 1] : std::string();
	case 3: return (size_t)(vars.misc.agents_ct - 1) < models.size() ? models[vars.misc.agents_ct - 1] : std::string();
	default: return {};
	}
};

void AgentChanger(int stage) {

	if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START && stage != FRAME_RENDER_END)
		return;

	static int originalIdx = 0;

	if (!csgo->local) {
		originalIdx = 0;
		return;
	}

	const auto model = GetModel(csgo->local->GetTeam());

	static auto addr = (uintptr_t)(csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		str("E8 ? ? ? ? 89 87 ? ? ? ? 6A")) + 1);

	static std::add_pointer_t<const char** __fastcall(const char* playerModelName)> getPlayerViewmodelArmConfigForPlayerModel =
		relativeToAbsolute<decltype(getPlayerViewmodelArmConfigForPlayerModel)>(addr);

	if (!model.empty()) {
		if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START) {
			originalIdx = csgo->local->GetModelIndex();
			if (const auto modelprecache = interfaces.net_string_container->findTable(str("modelprecache"))) {
				modelprecache->addString(false, model.c_str());
				const auto viewmodelArmConfig = getPlayerViewmodelArmConfigForPlayerModel(model.c_str());
				modelprecache->addString(false, viewmodelArmConfig[2]);
				modelprecache->addString(false, viewmodelArmConfig[3]);
			}
		}
		int team = csgo->local->GetTeam();

		bool original_model = team == 2 && vars.misc.agents_t == 0
			|| team == 3 && vars.misc.agents_ct == 0;

		const auto idx = (original_model || stage == FRAME_RENDER_END) && originalIdx ? originalIdx : interfaces.models.model_info->GetModelIndex(model.c_str());

		csgo->local->SetModelIndex(idx);

		if (const auto ragdoll = interfaces.ent_list->GetClientEntityFromHandle(csgo->local->GetRagdollHandle()))
			ragdoll->SetModelIndex(idx);
	}
}

#define INVALID_EHANDLE_INDEX 0xFFFFFFFF

bool apply_knife_model(IBaseCombatWeapon* weapon, const char* model) {
	auto viewmodel = csgo->local->GetViewModel();

	if (!viewmodel)
		return false;

	static int offset = netvars.GetOffset(str("DT_BaseViewModel"), str("m_hWeapon"));
	auto m_hWeapon = *(CBaseHandle*)(uintptr_t(viewmodel) + offset);

	auto view_model_weapon = (IBaseCombatWeapon*)(interfaces.ent_list->GetClientEntityFromHandle(m_hWeapon));

	if (view_model_weapon != weapon)
		return false;

	viewmodel->GetModelIndex() = interfaces.models.model_info->GetModelIndex(model);

	return true;
}

bool apply_knife_skin(IBaseCombatWeapon* weapon, int item_definition_index, int paint_kit, int model_index, int entity_quality, float fallback_wear) {
	weapon->GetItemDefinitionIndex() = item_definition_index;
	weapon->FallbackPaintKit() = paint_kit;
	((IBasePlayer*)weapon)->GetModelIndex() = model_index;
	weapon->EntityQuality() = entity_quality;
	weapon->FallbackWear() = fallback_wear;
	weapon->preDataUpdate(0);
	return true;
}

void skinchanger() noexcept {
	//if (!cfg.skinchanger)
	//	return;
	//csgo->weapon = csgo->local->GetWeapon();
	//if (!csgo->weapon || !csgo->local)
	//	return;

	//auto model_bayonet = "models/weapons/v_knife_bayonet.mdl";
	//auto model_m9 = "models/weapons/v_knife_m9_bay.mdl";
	//auto model_karambit = "models/weapons/v_knife_karam.mdl";
	//auto model_bowie = "models/weapons/v_knife_survival_bowie.mdl";
	//auto model_butterfly = "models/weapons/v_knife_butterfly.mdl";
	//auto model_falchion = "models/weapons/v_knife_falchion_advanced.mdl";
	//auto model_flip = "models/weapons/v_knife_flip.mdl";
	//auto model_gut = "models/weapons/v_knife_gut.mdl";
	//auto model_huntsman = "models/weapons/v_knife_tactical.mdl";
	//auto model_shadow_daggers = "models/weapons/v_knife_push.mdl";
	//auto model_navaja = "models/weapons/v_knife_gypsy_jackknife.mdl";
	//auto model_stiletto = "models/weapons/v_knife_stiletto.mdl";
	//auto model_talon = "models/weapons/v_knife_widowmaker.mdl";
	//auto model_ursus = "models/weapons/v_knife_ursus.mdl";

	//auto index_bayonet = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
	//auto index_m9 = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
	//auto index_karambit = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_karam.mdl");
	//auto index_bowie = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_survival_bowie.mdl");
	//auto index_butterfly = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
	//auto index_falchion = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
	//auto index_flip = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_flip.mdl");
	//auto index_gut = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_gut.mdl");
	//auto index_huntsman = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_tactical.mdl");
	//auto index_shadow_daggers = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_push.mdl");
	//auto index_navaja = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_gypsy_jackknife.mdl");
	//auto index_stiletto = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_stiletto.mdl");
	//auto index_talon = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_widowmaker.mdl");
	//auto index_ursus = interfaces.models.model_info->GetModelIndex("models/weapons/v_knife_ursus.mdl");

	//for (auto Weapon : csgo->local->GetWeapons()) {
	//	if (csgo->weapon->IsKnife()) {
	//		apply_knife_model(Weapon, model_butterfly);
	//	}

	//	//Weapon->AccountID() = 0;
	//	//Weapon->FallbackSeed() = 661;
	//	//Weapon->ItemIDHigh() = -1; //-1
	//}
}

void UpdateViewmodel(IBasePlayer* player)
{
	static auto update_all_viewmodel_addons = reinterpret_cast<int(__fastcall*)(void*)>(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		str("55 8B EC 83 E4 ? 83 EC ? 53 8B D9 56 57 8B 03 FF 90 ? ? ? ? 8B F8 89 7C 24 ? 85 FF 0F 84 ? ? ? ? 8B 17 8B CF")));

	static auto get_viewmodel = reinterpret_cast<void* (__thiscall*)(void*, bool)>(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		str("55 8B EC 8B 45 08 53 8B D9 56 8B 84 83 ? ? ? ? 83 F8 FF 74 18 0F")));

	auto v10 = getvfunc<int(__thiscall*)(IBasePlayer*)>(player, 157)(player);
	if (!v10) return;

	auto v11 = getvfunc<int(__thiscall*)(IBasePlayer*)>(player, 267)(player);
	if (!v11) return;

	//fixed 
	{
		if (player->GetViewModel())
			update_all_viewmodel_addons(player->GetViewModel());
		return;
	}
	//fixed 

}

bool __fastcall Hooked_GetBool(void* pConVar, void* edx)
{
	static auto SvCheatsGetBool = g_pGetBoolHook->GetOriginal< SvCheatsGetBoolFn >(g_HookIndices[fnva1(hs::Hooked_GetBool.s().c_str())]);

	if (csgo->DoUnload)
		return SvCheatsGetBool(pConVar);

	static auto CAM_THINK = csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::retn_camera.s().c_str());
	if (_ReturnAddress() == CAM_THINK)
		return true;

	return SvCheatsGetBool(pConVar);
}

static bool enabledtp = false, check = false;

__forceinline void UpdateCSGOKeyBinds()
{
	if (g_Binds[bind_third_person].active)
	{
		if (!check)
			enabledtp = !enabledtp;
		check = true;
	}
	else
		check = false;


	csgo->SwitchAA = g_Binds[bind_aa_inverter].active;
}

struct ClientHitVerify_t {
	Vector pos;
	float time;
	float expires;
};

void __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage)
{
	static auto last_count = 0;
	static auto FrameStageNotify = g_pClientHook->GetOriginal< FrameStageNotifyFn >(g_HookIndices[fnva1(hs::Hooked_FrameStageNotify.s().c_str())]);

	csgo->updatelocalplayer();

	if (!interfaces.engine->IsConnected() || !interfaces.engine->IsInGame()) {
		csgo->mapChanged = true;
		csgo->is_connected = false;
		g_Resolver->ClearHitInfo();
	}
	else csgo->is_connected = true;

	if (curStage == FRAME_START) {
		g_Cache->FrameStageNotify();
		g_Visuals->StoreOtherInfo();
	}

	if (curStage == FRAME_RENDER_START) {
		g_Misc->PreverseKillFeed(csgo->round_start);

		if (csgo->round_start)
			csgo->round_start = false;

		bool disablePostProcess = vars.visuals.remove & 16;

		static auto mat_postprocess_enable = interfaces.cvars->FindVar(str("mat_postprocess_enable"));
		mat_postprocess_enable->m_fnChangeCallbacks.m_Size = 0;
		mat_postprocess_enable->SetValue(!disablePostProcess);
	}

	if (!csgo->local /*|| !csgo->local->isAlive()*/) {
		csgo->ForceOffAA = false;
		if (curStage == FRAME_NET_UPDATE_END) {
			for (int i = 1; i < 65; i++) {
				auto entity = interfaces.ent_list->GetClientEntity(i);
				if (entity != nullptr && entity->IsPlayer() && entity != csgo->local) {
					entity->GetClientSideAnims() = csgo->EnableBones = true;
				}
			}
			csgo->disable_dt = false;
		}
		return FrameStageNotify(curStage);
	}


	if (curStage == FRAME_RENDER_START)
	{
		if (csgo->game_rules == nullptr || csgo->mapChanged) {
			csgo->game_rules = **reinterpret_cast<CCSGameRules***>(csgo->Utils.FindPatternIDA(GetModuleHandleA(
				g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
				hs::game_rules.s().c_str()) + 0x1);
			for (int i = 0; i < 64; i++)
				g_Visuals->player_info[i].Reset();

		}
	}

	if (curStage == FRAME_RENDER_START) {
		if (csgo->mapChanged) csgo->mapChanged = false;
		g_Misc->Ragdolls();
	}

	AgentChanger(curStage);

	UpdateCSGOKeyBinds();

	if (interfaces.engine->IsConnected() && interfaces.engine->IsInGame())
	{
		if (curStage == FRAME_NET_UPDATE_END) {
			static auto r_jiggle_bones = interfaces.cvars->FindVar(str("r_jiggle_bones"));
			r_jiggle_bones->m_fnChangeCallbacks.m_Size = 0;
			if (r_jiggle_bones->GetInt() > 0)
				r_jiggle_bones->SetValue(0);

			g_Animfix->UpdatePlayers();
		}

		if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START) {
			if (csgo->local && csgo->local->isAlive()) {
				g_NetData->ApplyViewmodelValues();
			}
		}

		if (curStage == FRAME_RENDER_START)
		{
			g_Animfix->FixPvs();

			if (csgo->local->isAlive()) {
				auto& ImpactList = *(CUtlVector< ClientHitVerify_t >*)((uintptr_t)csgo->local + 0xBC00);

				if (vars.visuals.bullet_impact) {
					const auto& size = vars.visuals.impacts_size / 10.f;
					for (auto i = ImpactList.Count(); i > last_count; i--) {
						interfaces.debug_overlay->add_box_overlay(ImpactList[i - 1].pos, Vector(-size, -size, -size), Vector(size, size, size), Vector(0, 0, 0),
							vars.visuals.client_impact_color[0],
							vars.visuals.client_impact_color[1],
							vars.visuals.client_impact_color[2], vars.visuals.client_impact_color[3], 4.f);
					}

					if (ImpactList.Count() != last_count)
						last_count = ImpactList.Count();

					ImpactList.RemoveAll();
				}

				bool fr = csgo->game_rules->IsFreezeTime()
					|| csgo->local->HasGunGameImmunity()
					|| csgo->local->GetFlags() & FL_FROZEN;

				if (!fr) {
					csgo->local->GetPlayerAnimState()->m_abs_yaw = csgo->last_gfy;
					csgo->local->SetAbsAngles(Vector(0, csgo->local->GetPlayerAnimState()->m_abs_yaw, 0));
				}

				UpdateViewmodel(csgo->local);
				if (vars.visuals.remove & 4)
					csgo->local->GetFlashDuration() = 0.f;
				if (csgo->weapon)
					csgo->scoped = csgo->weapon->isSniper() && csgo->local->IsScoped();
				else
					csgo->scoped = false;
				csgo->zoom_level = csgo->weapon->GetZoomLevel() * csgo->scoped;
			}
			interfaces.input->m_fCameraInThirdPerson = false;

			g_BulletTracer->Proceed();
		}

		//if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		//	skinchanger();

		csgo->disable_dt = false;
		csgo->ForceOffAA = false;
	}
	else {
		csgo->disable_dt = false;
		csgo->mapChanged = true;
		csgo->need_to_recharge = false;
		csgo->skip_ticks = 0;
	}

	FrameStageNotify(curStage);

	if (curStage == FRAME_NET_UPDATE_END)
		g_NetData->Apply();

	if (curStage == FRAME_RENDER_START) {
		if (csgo->local && csgo->local->isAlive())
			interfaces.prediction->SetLocalViewAngles(csgo->FakeAngle);
	}
}

bool __fastcall Hooked_ShouldDrawFog(void* ecx, void* edx) {
	return !(vars.visuals.remove & 32);
}

bool __fastcall Hooked_ShouldDrawShadow(void*, uint32_t) {
	return !(vars.visuals.remove & 64);
}
