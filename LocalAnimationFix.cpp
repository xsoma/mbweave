#include "AnimationFix.h"

void CAnimationFix::UpdateFakeState()
{
	if ((!interfaces.engine->IsConnected() && !interfaces.engine->IsInGame()) || !csgo->local)
		return;

	if (!csgo->local->isAlive())
		return;

	static CBaseHandle* selfhandle = nullptr;
	static float spawntime = csgo->local->GetSpawnTime();

	auto alloc = FakeAnimstate == nullptr;
	auto change = !alloc && selfhandle != &csgo->local->GetRefEHandle();
	auto reset = !alloc && !change && csgo->local->GetSpawnTime() != spawntime;

	if (change) {
		memset(&FakeAnimstate, 0, sizeof(FakeAnimstate));
		selfhandle = (CBaseHandle*)&csgo->local->GetRefEHandle();
	}
	if (reset) {
		csgo->local->ResetAnimationState(FakeAnimstate);
		spawntime = csgo->local->GetSpawnTime();
	}

	if (alloc || change) {
		FakeAnimstate = reinterpret_cast<CCSGOPlayerAnimState*>(interfaces.memalloc->Alloc(sizeof(CCSGOPlayerAnimState)));
		if (FakeAnimstate)
			csgo->local->CreateAnimationState(FakeAnimstate);
	}

	if (!FakeAnimstate)
		return;

	if (FakeAnimstate->m_last_update_frame == interfaces.global_vars->framecount)
		FakeAnimstate->m_last_update_frame -= 1;

	CAnimationLayer layers[13] = {};
	std::array<float, 24> poses = {};

	csgo->local->ParseAnimOverlays(layers);
	csgo->local->ParsePoseParameter(poses);

	if (csgo->send_packet)
	{
		csgo->local->UpdateAnimationState(FakeAnimstate, csgo->FakeAngle); // update animstate

		csgo->local->InvalidateBoneCache();
		csgo->UpdateMatrixLocal = true;
		csgo->local->SetupBones(csgo->fakematrix, 256, 0x7FF00, interfaces.global_vars->curtime);// setup matrix
		csgo->UpdateMatrixLocal = false;

		for (auto& i : csgo->fakematrix)
		{
			i[0][3] -= csgo->local->GetRenderOrigin().x;
			i[1][3] -= csgo->local->GetRenderOrigin().y;
			i[2][3] -= csgo->local->GetRenderOrigin().z;
		}
	}

	csgo->local->SetAnimOverlays(layers);
	csgo->local->SetPoseParameter(poses);

	csgo->animstate = FakeAnimstate;
}

void CAnimationFix::UpdateRealState() {
	if ((!interfaces.engine->IsConnected() && !interfaces.engine->IsInGame()) || !csgo->local)
		return;

	if (!csgo->local->isAlive())
		return;

	static CBaseHandle* selfhandle = nullptr;
	static float spawntime = csgo->local->GetSpawnTime();

	auto alloc = RealAnimstate == nullptr;
	auto change = !alloc && selfhandle != &csgo->local->GetRefEHandle();
	auto reset = !alloc && !change && csgo->local->GetSpawnTime() != spawntime;

	if (change) {
		memset(&RealAnimstate, 0, sizeof(RealAnimstate));
		selfhandle = (CBaseHandle*)&csgo->local->GetRefEHandle();
	}
	if (reset) {
		csgo->local->ResetAnimationState(RealAnimstate);
		spawntime = csgo->local->GetSpawnTime();
	}

	if (alloc || change) {
		RealAnimstate = reinterpret_cast<CCSGOPlayerAnimState*>(interfaces.memalloc->Alloc(sizeof(CCSGOPlayerAnimState)));
		if (RealAnimstate)
			csgo->local->CreateAnimationState(RealAnimstate);
	}

	if (!RealAnimstate)
		return;

	if (RealAnimstate->m_last_update_frame == interfaces.global_vars->framecount)
		RealAnimstate->m_last_update_frame -= 1;

	Vector old_ang = *(Vector*)((DWORD)csgo->local + 0x31D8);
	
	*(Vector*)((DWORD)csgo->local + 0x31D8) = csgo->FakeAngle;

	csgo->ShouldUpdate = true;
	csgo->local->UpdateAnimationState(RealAnimstate, csgo->CurAngle);
	csgo->ShouldUpdate = false;

	if (csgo->send_packet) {
		csgo->local->ParseAnimOverlays(csgo->layers);
		csgo->local->ParsePoseParameter(csgo->poses);

		if (RealAnimstate)
			csgo->last_gfy = RealAnimstate->m_abs_yaw;

		if (FakeAnimstate && RealAnimstate)
			csgo->desync_angle = clamp(Math::NormalizeYaw(Math::AngleDiff(FakeAnimstate->m_abs_yaw, RealAnimstate->m_abs_yaw)), -58.f, 58.f);
	}

	csgo->layers[12].m_flWeight = FLT_EPSILON;

	*(Vector*)((DWORD)csgo->local + 0x31D8) = old_ang;
}
