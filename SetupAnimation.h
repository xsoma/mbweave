#pragma once
#include "Hooks.h"

void __fastcall Hooked_StandardBlendingRules(IBasePlayer* entity, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, float curTime, int boneMask)
{
	static auto original = hooker::h.original(&Hooked_StandardBlendingRules);

	if (!entity->IsPlayer())
		return original(entity, edx, hdr, pos, q, curTime, boneMask);

	auto backup_effects = entity->GetEffects();
	entity->GetEffects() |= 8;
	original(entity,edx, hdr, pos, q, curTime, boneMask);
	entity->GetEffects() = backup_effects;
}

bool __fastcall Hooked_SetupBones(void* ecx, void* edx, matrix* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
{
	static auto original = hooker::h.original(&Hooked_SetupBones);

	if (csgo->DoUnload)
		return original(ecx, edx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);

	auto entity = (IBasePlayer*)((uintptr_t)ecx - 4);
	if (csgo->game_rules->IsFreezeTime()
		|| entity == nullptr || csgo->local == nullptr || !pBoneToWorldOut 
		|| !entity->IsPlayer()
		|| (entity->EntIndex() != interfaces.engine->GetLocalPlayer()
			&& entity->GetTeam() == csgo->local->GetTeam()))
		return original(ecx, edx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);

	auto state = entity->GetPlayerAnimState();
	if (!state)
		return original(ecx, edx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);

	if (entity->EntIndex() == interfaces.engine->GetLocalPlayer())
	{
		entity->GetAnimOverlays()[12].m_flWeight = FLT_EPSILON;
		return original(ecx, edx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
	}
	else {
		if (!csgo->UpdateMatrix)
		{
			memcpy(pBoneToWorldOut, entity->GetBoneCache().Base(), sizeof(matrix) * nMaxBones);
			return true;
		}
		return original(ecx, edx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
	}
}

bool __fastcall Hooked_ShouldSkipAnimFrame(void* ecx, void* edx)
{
	static auto original = hooker::h.original(&Hooked_ShouldSkipAnimFrame);

	auto player = (IBasePlayer*)ecx;
	if (!player)
		return original(ecx, edx);

	if (player->EntIndex() == interfaces.engine->GetLocalPlayer())
		return false;
	else
		return original(ecx, edx);
}

void __fastcall Hooked_DoExtraBoneProcessing(IBasePlayer* player, uint32_t, CStudioHdr* hdr, Vector* pos, Quaternion* q,
	const matrix& mat, uint8_t* bone_computed, void* context)
{
	return;
}

void __fastcall Hooked_UpdateClientSideAnimation(IBasePlayer* player, uint32_t edx)
{
	static auto original = hooker::h.original(&Hooked_UpdateClientSideAnimation);

	if (csgo->DoUnload || !player || !csgo->local || !csgo->local->isAlive() || (player->GetTeam() == csgo->local->GetTeam() && player != csgo->local)
		|| csgo->game_rules->IsFreezeTime()
		|| csgo->local->HasGunGameImmunity()
		|| csgo->local->GetFlags() & FL_FROZEN) {
		player->GetClientSideAnims() = true;
		return original(player, edx);
	}

	if (csgo->local == player) {
		if (csgo->ShouldUpdate)
			original(player, edx);
		else {
			std::array<float, 24> poses = {};
			CAnimationLayer layers[13] = {};

			// backup old data
			player->ParseAnimOverlays(layers);
			player->ParsePoseParameter(poses);

			// set last recieved data on bones
			player->SetAnimOverlays(csgo->layers);
			player->SetPoseParameter(csgo->poses);

			// apply foot angle on bones
			player->GetPlayerAnimState()->m_abs_yaw = csgo->last_gfy;
			player->SetAbsAngles(Vector(0, player->GetPlayerAnimState()->m_abs_yaw, 0));
			player->SetupBones(nullptr, -1, 0x7FF00, interfaces.global_vars->curtime);

			// restore old data so it won't fuck up
			player->SetAnimOverlays(layers);
			player->SetPoseParameter(poses);
		}
	}
	else {
		if (csgo->EnableBones) 
			original(player, edx);
	}
}

void __fastcall Hooked_ModifyEyePosition(CCSGOPlayerAnimState* state, void* edx, const Vector& vec) {

	static auto original = hooker::h.original(&Hooked_ModifyEyePosition);

	if (csgo->DoUnload)
		return original(state, edx, vec);

	IBasePlayer* ent = (IBasePlayer*)state->m_player;
	if (!ent)
		return original(state, edx, vec);

	if (csgo->local && ent != csgo->local)
		original(state, edx, vec);
	else
		return;
}

void __fastcall Hooked_CalcViewModelView(void* ecx, void* edx, IBasePlayer* owner, Vector& eyePos, Vector& eyeAngs) {
	static auto original = hooker::h.original(&Hooked_CalcViewModelView);

	if (csgo->DoUnload)
		return original(ecx,edx, owner, eyePos, eyeAngs);

	if (!csgo->is_connected || !csgo->local || !owner || !owner->IsPlayer() || !csgo->is_local_alive || owner != csgo->local)
		return original(ecx, edx, owner, eyePos, eyeAngs);

	if (csgo->fake_duck)
		eyePos = owner->GetRenderOrigin() + Vector(0.0f, 0.0f, interfaces.game_movement->GetPlayerViewOffset(false).z + 0.064f);
	
	original(ecx, edx, owner, eyePos, eyeAngs);
}

void __fastcall Hooked_AddViewModelBob(void* ecx, void* edx, IBaseViewModel* model, Vector& origin, Vector& angles) {
	static auto original = hooker::h.original(&Hooked_AddViewModelBob);

	if (csgo->DoUnload)
		return original(ecx, edx, model, origin, angles);

	if (vars.visuals.remove & 256)
		return;

	original(ecx, edx, model, origin, angles);
}