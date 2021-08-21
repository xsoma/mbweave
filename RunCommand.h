#pragma once
#include <intrin.h>
#include "EnginePrediction.h"

void __fastcall Hooked_RunCommand(void* ecx, void* edx, IBasePlayer* player, CUserCmd* ucmd, IMoveHelper* moveHelper)
{
	static auto RunCommand = g_pPredictHook->GetOriginal< RunCommandFn >(g_HookIndices[fnva1(hs::Hooked_RunCommand.s().c_str())]);

	if (csgo->DoUnload || player == nullptr || csgo->local == nullptr 
		|| player->EntIndex() != interfaces.engine->GetLocalPlayer() ||  !csgo->local->isAlive())
		return RunCommand(ecx, player, ucmd, moveHelper);

	if (!interfaces.engine->IsConnected() || !interfaces.engine->IsInGame())
		return RunCommand(ecx, player, ucmd, moveHelper);

	if (ucmd->tick_count > interfaces.global_vars->tickcount * 2) //-V807
	{
		ucmd->hasbeenpredicted = true;
		player->SetAbsOrigin(player->GetOrigin());
		if (!interfaces.engine->IsPaused())
			player->GetTickBasePtr()++;
		return;
	}
	
	RunCommand(ecx, player, ucmd, moveHelper);

	g_NetData->Store();
}

bool __fastcall Hooked_InPrediction(CPrediction* prediction, uint32_t)
{
	static auto InPrediction = g_pPredictHook->GetOriginal< InPredictionFn >(g_HookIndices[fnva1(hs::Hooked_InPrediction.s().c_str())]);

	if (csgo->DoUnload)
		return InPrediction(prediction);

	static const auto return_to_maintain_sequence_transitions = csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::seq_transition.s().c_str());

	if (_ReturnAddress() == (uint32_t*)return_to_maintain_sequence_transitions)
		return false;

	return InPrediction(prediction);
}

void __fastcall Hooked_ProcessMovement(void* ecx, void* edx, IBasePlayer* ent, CMoveData* move_data) {
	static auto ProcessMovement = g_pMovementHook->GetOriginal< void(__thiscall*)(void*, IBasePlayer*, CMoveData*) >(g_HookIndices[fnva1(hs::Hooked_ProcessMovement.s().c_str())]);
	move_data->m_bGameCodeMovedPlayer = false; // fix shit when you jump and movement are fucked up (credits to l3d451r7)
	ProcessMovement(ecx, ent, move_data);
}