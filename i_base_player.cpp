#pragma once
#include "Hooks.h"
#include "netvar_manager.h"

unsigned int FindInDataMap(datamap_t *pMap, const char *name);

bool CGameTrace::DidHitWorld() const {
	return m_pEnt == interfaces.ent_list->GetClientEntity(0);
}

bool CGameTrace::DidHitNonWorldEntity() const {
	return m_pEnt != nullptr && !DidHitWorld();
}

bool IBasePlayer::ComputeHitboxSurroundingBox(Vector* mins, Vector* maxs) {
	using compute_hitbox_surrounding_box_t = bool(__thiscall*)(void*, Vector*, Vector*);
	static auto compute_hitbox_surrounding_box 
		= reinterpret_cast<compute_hitbox_surrounding_box_t>(csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
			str("55 8B EC 83 E4 F8 81 EC 24 04 00 00 ? ? ? ? ? ?")));
	if (!compute_hitbox_surrounding_box)
		return false;

	return compute_hitbox_surrounding_box(this, mins, maxs);
}

bool& IBasePlayer::TargetSpotted() {
	return *(bool*)(uintptr_t(this) + 0x93D);
}

CStudioHdr* IBasePlayer::GetModelPtr()
{
	return *(CStudioHdr**)((uintptr_t)this + 0x294C);
}

void IBasePlayer::ParsePoseParameter(std::array<float, 24>& param) {

	std::copy(this->m_flPoseParameter().begin(), this->m_flPoseParameter().end(),
		param.begin());
}

void IBasePlayer::SetPoseParameter(std::array<float, 24> param) {

	std::copy(param.begin(), param.end(),
		this->m_flPoseParameter().begin());
}

void IBasePlayer::ParseAnimOverlays(CAnimationLayer* anim) {

	std::memcpy(anim, this->GetAnimOverlays(),
		(sizeof(CAnimationLayer) * this->GetNumAnimOverlays()));
}

void IBasePlayer::SetAnimOverlays(CAnimationLayer* anim) {

	std::memcpy(this->GetAnimOverlays(), anim,
		(sizeof(CAnimationLayer) * this->GetNumAnimOverlays()));
}

void IBasePlayer::AttachmentHelper()
{
	using AttachmentHelperFn = void(__thiscall*)(IBasePlayer*, CStudioHdr*);
	static AttachmentHelperFn DoAttachments = (AttachmentHelperFn)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		str("55 8B EC 83 EC 48 53 8B 5D 08 89 4D F4")));
	DoAttachments(this, this->GetModelPtr());
};

int& IBasePlayer::GetPlayerState()
{
	static auto m_iPlayerState = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), str("m_iPlayerState"));
	return *(int*)(uintptr_t(this) + m_iPlayerState);
}

bool& IBasePlayer::IsDefusing()
{
	static int m_bIsDefusing = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), str("m_bIsDefusing"));
	return *(bool*)(uintptr_t(this) + m_bIsDefusing);
}

bool& IBasePlayer::WaitForNoAttack()
{
	static auto m_bWaitForNoAttack = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), str("m_bWaitForNoAttack"));
	return *(bool*)(uintptr_t(this) + m_bWaitForNoAttack);
}

CBoneAccessor* IBasePlayer::GetBoneAccessor() {
	static int offset = netvars.GetOffset(hs::DT_BaseAnimating.s().c_str(), hs::m_nForceBone.s().c_str());
	return (CBoneAccessor*)((uintptr_t)this + offset + 0x1C);
}

CUtlVector<matrix>& IBasePlayer::GetBoneCache() {
	static auto m_CachedBoneData = *(DWORD*)(csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::bone_cache.s().c_str()) + 0x2) + 0x4;
	return *(CUtlVector<matrix>*)(uintptr_t(this) + m_CachedBoneData);
}

void IBasePlayer::SetBoneCache(matrix* m) {
	auto cache = GetBoneCache();
	memcpy(cache.Base(), m, sizeof(matrix) * cache.Count());
}

__forceinline void IBasePlayer::ShootPos(Vector* vec)
{
	static auto Pattern = csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::shoot_pos.s().c_str());

	typedef float*(__thiscall* ShootPosFn)(void*, Vector*);

	static auto Fn = (ShootPosFn)Pattern;

	if (!Fn)
		return;

	Fn(this, vec);
}

void IBasePlayer::UpdateVisibilityAllEntities()
{
	static DWORD callInstruction = (DWORD)csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::visibility.s().c_str()); // get the instruction address

	static uintptr_t* update_visibility_all_entities = nullptr;
	if (update_visibility_all_entities == nullptr) {
		static DWORD relativeAddress = *(DWORD*)(callInstruction + 1); // read the rel32
		static DWORD nextInstruction = callInstruction + 5; // get the address of next instruction
		update_visibility_all_entities = (uintptr_t*)(nextInstruction + relativeAddress); // our function address will be nextInstruction + relativeAddress
	}
	else
		reinterpret_cast<void(__thiscall*)(void*)>(update_visibility_all_entities)(this);
}

Vector IBasePlayer::GetEyePosition()
{
	return GetOrigin() + GetVecViewOffset();
}

void IBasePlayer::DrawServerHitboxes() {
	auto duration = interfaces.global_vars->interval_per_tick * 2.0f;

	auto serverPlayer = GetServerEdict();
	if (serverPlayer) {
		static auto pCall = (uintptr_t*)(csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::server_dll.s().c_str())]().c_str()),
			hs::server_hitbox.s().c_str()));
		float fDuration = duration;

		__asm
		{
			pushad
			movss xmm1, fDuration
			push 1 //bool monoColor
			mov ecx, serverPlayer
			call pCall
			popad
		}
	}
}

__forceinline uint8_t* IBasePlayer::GetServerEdict() {
	static uintptr_t pServerGlobals = **(uintptr_t**)(csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::server_dll.s().c_str())]().c_str()),
		hs::server_edict.s().c_str()) + 0x2);
	int iMaxClients = *(int*)((uintptr_t)pServerGlobals + 0x18);
	int index = GetIndex();
	if (index > 0 && iMaxClients >= 1) {
		if (index <= iMaxClients) {
			int v10 = index * 16;
			uintptr_t v11 = *(uintptr_t*)(pServerGlobals + 96);
			if (v11) {
				if (!((*(uintptr_t*)(v11 + v10) >> 1) & 1)) {
					uintptr_t v12 = *(uintptr_t*)(v10 + v11 + 12);
					if (v12) {
						uint8_t* pReturn = nullptr;

						// abusing asm is not good
						__asm
						{
							pushad
							mov ecx, v12
							mov eax, dword ptr[ecx]
							call dword ptr[eax + 0x14]
							mov pReturn, eax
							popad
						}

						return pReturn;
					}
				}
			}
		}
	}
	return nullptr;
}

void IBasePlayer::InvalidateBoneCache()
{
	static DWORD addr = (DWORD)csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::invalidate_bone_cache.s().c_str());

	[&] { // anti eblan always on
		*reinterpret_cast<int*>((uintptr_t)this + 0xA30) = interfaces.global_vars->framecount;
		*reinterpret_cast<int*>((uintptr_t)this + 0xA28) = 0;

		unsigned long g_iModelBoneCounter = **(unsigned long**)(addr + 10);
		*reinterpret_cast<unsigned int*>((DWORD)this + 0x2924) = 0xFF7FFFFF;
		*reinterpret_cast<unsigned int*>((DWORD)this + 0x2690) = (g_iModelBoneCounter - 1);
	}();
}

bool IBasePlayer::IsValid()
{
	if (!this->IsPlayer()
		|| this->IsDormant()
		|| !this->isAlive()
		|| this == csgo->local
		|| this->GetTeam() == csgo->local->GetTeam()
		|| this->HasGunGameImmunity()
		|| this->GetFlags() & FL_FROZEN)
		return false;

	return true;
}

Vector& IBasePlayer::GetMins() {
	static int offset = netvars.GetOffset(hs::DT_BaseEntity.s().c_str(), hs::m_vecMins.s().c_str());
	return *(Vector*)((DWORD)this + offset);
}

Vector& IBasePlayer::GetMaxs() {
	static int offset = netvars.GetOffset(hs::DT_BaseEntity.s().c_str(), hs::m_vecMaxs.s().c_str());
	return *(Vector*)((DWORD)this + offset);
}

__forceinline HANDLE IBasePlayer::GetObserverTargetHandle()
{
	static int m_hObserverTarget = netvars.GetOffset(hs::DT_BasePlayer.s().c_str(), hs::m_hObserverTarget.s().c_str());
	return *(HANDLE*)((uintptr_t)this + m_hObserverTarget);
}

Vector& IBasePlayer::GetAbsVelocity()
{
	static unsigned int offset = FindInDataMap(GetPredDescMap(), hs::m_vecAbsVelocity.s().c_str());
	return *(Vector*)((uintptr_t)this + offset);
}

int& IBasePlayer::GetGroundEntity()
{
	static unsigned int offset = FindInDataMap(GetPredDescMap(), hs::m_hGroundEntity.s().c_str());
	return *(int*)((uintptr_t)this + offset);
}

float IBasePlayer::GetOldSimulationTime()
{
	static int m_flSimulationTime = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_flSimulationTime.s().c_str()) + 4;
	return *(float*)((DWORD)this + m_flSimulationTime);
}

float IBasePlayer::CameFromDormantTime()
{
	return csgo->dormant_time[GetIndex()];
}

__forceinline player_info_t IBasePlayer::GetPlayerInfo()
{
	player_info_t pinfo;
	interfaces.engine->GetPlayerInfo(GetIndex(), &pinfo);
	return pinfo;
}

float &IBasePlayer::HealthShotBoostExpirationTime()
{
	static int offset = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_flHealthShotBoostExpirationTime.s().c_str());
	return *(float*)((DWORD)this + offset);
}

void IBasePlayer::ResetAnimationState(CCSGOPlayerAnimState *state)
{
	if (!state)
		return;

	using ResetAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*);
	static auto ResetAnimState = (ResetAnimState_t)csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::reset_animstate.s().c_str());
	if (!ResetAnimState)
		return;

	ResetAnimState(state);
}

void IBasePlayer::CreateAnimationState(CCSGOPlayerAnimState *state)
{
	using CreateAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*, IBasePlayer*);
	static auto CreateAnimState = (CreateAnimState_t)csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::create_animstate.s().c_str());
	if (!CreateAnimState)
		return;

	CreateAnimState(state, this);
}

void IBasePlayer::UpdateAnimationState(CCSGOPlayerAnimState *state, Vector angle)
{
	using fn = void(__vectorcall*)(void*, void*, float, float, float, void*);
	static auto ret = reinterpret_cast<fn>(csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::update_animstate.s().c_str()));

	if (!ret)
		return;

	ret(state, NULL, NULL, angle.y, angle.x, NULL);
}

IBasePlayer* IBasePlayer::GetDefuser() {
	static int offset = netvars.GetOffset(str("DT_PlantedC4"), str("m_hBombDefuser"));
	DWORD defuser = *(DWORD*)((DWORD)this + offset);
	return (IBasePlayer*)interfaces.ent_list->GetClientEntityFromHandle(defuser);
}

float IBasePlayer::GetTimerLength() {
	static std::uintptr_t m_flTimerLength = netvars.GetOffset(str("DT_PlantedC4"), str("m_flTimerLength"));
	return *(float*)((std::uintptr_t)this + m_flTimerLength);
}

float IBasePlayer::GetDefuseCooldown() {
	static std::uintptr_t m_flDefuseCountDown = netvars.GetOffset(str("DT_PlantedC4"), str("m_flDefuseCountDown"));
	return *(float*)((std::uintptr_t)this + m_flDefuseCountDown);
}

float IBasePlayer::GetC4Blow() {
	static std::uintptr_t m_flC4Blow = netvars.GetOffset(str("DT_PlantedC4"), str("m_flC4Blow"));
	return *(float*)((std::uintptr_t)this + m_flC4Blow);
}

float IBasePlayer::GetBombTicking() {
	static std::uintptr_t m_bBombTicking = netvars.GetOffset(str("DT_PlantedC4"), str("m_bBombTicking"));
	return *(float*)((std::uintptr_t)this + m_bBombTicking);
}

bool IBasePlayer::IsBombDefused() {
	static std::uintptr_t m_bBombDefused = netvars.GetOffset(str("DT_PlantedC4"), str("m_bBombDefused"));
	return *(bool*)((std::uintptr_t)this + m_bBombDefused);
}

int* IBasePlayer::GetButtons()
{
	static std::uintptr_t m_nButtons = FindInDataMap(this->GetPredDescMap(), hs::m_nButtons.s().c_str());
	return (int*)((std::uintptr_t)this + m_nButtons);
}

int& IBasePlayer::GetButtonLast()
{
	static std::uintptr_t m_afButtonLast = FindInDataMap(this->GetPredDescMap(), (hs::m_afButtonLast.s().c_str()));
	return *(int*)((std::uintptr_t)this + m_afButtonLast);
}

int& IBasePlayer::GetButtonPressed()
{
	static std::uintptr_t m_afButtonPressed = FindInDataMap(this->GetPredDescMap(), (hs::m_afButtonPressed.s().c_str()));
	return *(int*)((std::uintptr_t)this + m_afButtonPressed);
}

int& IBasePlayer::GetButtonReleased()
{
	static std::uintptr_t m_afButtonReleased = FindInDataMap(this->GetPredDescMap(), (hs::m_afButtonReleased.s().c_str()));
	return *(int*)((std::uintptr_t)this + m_afButtonReleased);
}

int IBasePlayer::m_fireCount() {
	static int m_fireCount = netvars.GetOffset(str("DT_Inferno"), str("m_fireCount"));
	return *reinterpret_cast<int*>(uintptr_t(this) + m_fireCount);
}

bool* IBasePlayer::m_bFireIsBurning() {
	static int m_bFireIsBurning = netvars.GetOffset(str("DT_Inferno"), str("m_bFireIsBurning"));
	return reinterpret_cast<bool*>(uintptr_t(this) + m_bFireIsBurning);
}

int* IBasePlayer::m_fireXDelta() {
	static int m_fireXDelta = netvars.GetOffset(str("DT_Inferno"), str("m_fireXDelta"));
	return reinterpret_cast<int*>(uintptr_t(this) + m_fireXDelta);
}

int* IBasePlayer::m_fireYDelta() {
	static int m_fireYDelta = netvars.GetOffset(str("DT_Inferno"), str("m_fireYDelta"));
	return reinterpret_cast<int*>(uintptr_t(this) + m_fireYDelta);
}

int* IBasePlayer::m_fireZDelta() {
	static int m_fireZDelta = netvars.GetOffset(str("DT_Inferno"), str("m_fireZDelta"));
	return reinterpret_cast<int*>(uintptr_t(this) + m_fireZDelta);
}

bool IBasePlayer::m_bDidSmokeEffect() {
	static int m_bDidSmokeEffect = netvars.GetOffset(str("DT_SmokeGrenadeProjectile"), str("m_bDidSmokeEffect"));
	return *reinterpret_cast<bool*>(uintptr_t(this) + m_bDidSmokeEffect);
}

int IBasePlayer::m_nSmokeEffectTickBegin() {
	static int m_nSmokeEffectTickBegin = netvars.GetOffset(str("DT_SmokeGrenadeProjectile"), str("m_nSmokeEffectTickBegin"));
	return *reinterpret_cast<int*>(uintptr_t(this) + m_nSmokeEffectTickBegin);
}

int IBasePlayer::GetButtonDisabled()
{
	return *(int*)((std::uintptr_t)this + 0x3330);
}

int IBasePlayer::GetButtonForced()
{
	return *(int*)((std::uintptr_t)this + 0x3334);
}

int* IBasePlayer::GetNextThinkTick()
{
	static int nNextThinkTick = netvars.GetOffset(hs::DT_LocalPlayerExclusive.s().c_str(), hs::nNextThinkTick.s().c_str());
	return (int*)((std::uintptr_t)this + nNextThinkTick);
}

bool IBasePlayer::PhysicsRunThink(int unk01)
{
	using PhysicsRunThinkFn = bool(__thiscall*)(void*, int);
	static auto oPhysicsRunThink = (PhysicsRunThinkFn)csgo->Utils.FindPatternIDA((GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str())),
		hs::physics_run_think.s().c_str());
	return oPhysicsRunThink(this, unk01);
}

int& IBaseCombatWeapon::AccountID() {
	static int offset = netvars.GetOffset(str("DT_BaseAttributableItem"), str("m_iAccountID"));
	return *(int*)((uintptr_t)this + offset);
}

int& IBaseCombatWeapon::ItemIDHigh() {
	static int offset = netvars.GetOffset(str("DT_BaseAttributableItem"), str("m_iItemIDHigh"));
	return *(int*)((uintptr_t)this + offset);
}

int& IBaseCombatWeapon::EntityQuality() {
	static int offset = netvars.GetOffset(str("DT_BaseAttributableItem"), str("m_iEntityQuality"));
	return *(int*)((uintptr_t)this + offset);
}

char* IBaseCombatWeapon::CustomName() {
	static int offset = netvars.GetOffset(str("DT_BaseAttributableItem"), str("m_szCustomName"));
	return (char*)((uintptr_t)this + offset);
}

unsigned int& IBaseCombatWeapon::FallbackPaintKit() {
	static int offset = netvars.GetOffset(str("DT_BaseAttributableItem"), str("m_nFallbackPaintKit"));
	return *(unsigned int*)((uintptr_t)this + offset);
}

float& IBaseCombatWeapon::FallbackSeed() {
	static int offset = netvars.GetOffset(str("DT_BaseAttributableItem"), str("m_nFallbackSeed"));
	return *(float*)((uintptr_t)this + offset);
}

int& IBaseCombatWeapon::FallbackWear() {
	static int offset = netvars.GetOffset(str("DT_BaseAttributableItem"), str("m_flFallbackWear"));
	return *(int*)((uintptr_t)this + offset);
}
void IBaseCombatWeapon::preDataUpdate(int updateType) {
	return getvfunc<void(__thiscall*)(void*, int)>(this, 6)(this + sizeof(uintptr_t) * 2, updateType);
}
void IBaseCombatWeapon::postDataUpdate(int updateType) {
	return getvfunc<void(__thiscall*)(void*, int)>(this, 7)(this + sizeof(uintptr_t) * 2, updateType);
}
unsigned int& IBaseCombatWeapon::FallbackStatTrak() {
	static int offset = netvars.GetOffset(str("DT_BaseAttributableItem"), str("m_nFallbackStatTrak"));
	return *(unsigned int*)((uintptr_t)this + offset);
}

bool& IBaseCombatWeapon::Initialized() {
	static int offset = netvars.GetOffset(str("DT_BaseAttributableItem"), str("m_bInitialized"));
	return *(bool*)((uintptr_t)this + offset);
}

int IBasePlayer::GetRagdollHandle() {
	static auto m_hRagdoll = netvars.GetOffset(str("DT_CSPlayer"), str("m_hRagdoll"));
	return *(int*)(uintptr_t(this) + m_hRagdoll);
}

void IBasePlayer::SetModelIndex(int model) {
	typedef void(__thiscall* oSetModelIndex)(PVOID, int);
	return getvfunc< oSetModelIndex >(this, 75)(this, model);
}

int &IBasePlayer::GetModelIndex() {
	static auto m_nModelIndex = netvars.GetOffset(str("DT_BaseEntity"), str("m_nModelIndex"));
	return *(int*)(uintptr_t(this) + m_nModelIndex);
}

void IBasePlayer::SetAbsAngles(const Vector &angles)
{
	using SetAbsAnglesFn = void(__thiscall*)(void*, const Vector &angles);
	static SetAbsAnglesFn SetAbsAngles = (SetAbsAnglesFn)csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::set_abs_angles.s().c_str());
	SetAbsAngles(this, angles);
}

void IBasePlayer::SetAbsOrigin(const Vector &origin)
{
	using SetAbsOriginFn = void(__thiscall*)(void*, const Vector &origin);
	static SetAbsOriginFn SetAbsOrigin = (SetAbsOriginFn)csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::set_abs_origin.s().c_str());
	SetAbsOrigin(this, origin);
}

float_t IBasePlayer::m_surfaceFriction()
{
	static unsigned int _m_surfaceFriction = FindInDataMap(GetPredDescMap(), hs::m_surfaceFriction.s().c_str());
	return *(float_t*)((uintptr_t)this + _m_surfaceFriction);
}

__forceinline datamap_t *IBasePlayer::GetDataDescMap()
{
	typedef datamap_t*(__thiscall *o_GetPredDescMap)(void*);
	return getvfunc<o_GetPredDescMap>(this, 15)(this);
}

__forceinline datamap_t *IBasePlayer::GetPredDescMap()
{
	typedef datamap_t*(__thiscall *o_GetPredDescMap)(void*);
	return getvfunc<o_GetPredDescMap>(this, 17)(this);
}

bool IBasePlayer::IsWeapon()
{
	typedef bool (__thiscall* Fn)(void*);
	return (getvfunc<Fn>(this, 165))(this);
}

std::array<float, 24> &IBasePlayer::m_flPoseParameter()
{
	static int pos_par_hh = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_flPoseParameter.s().c_str());
	return *reinterpret_cast<std::array<float, 24>*>(reinterpret_cast<uintptr_t>(this) + pos_par_hh);
}

Vector& IBasePlayer::GetVecViewOffset()
{
	static int iOffset = netvars.GetOffset(hs::DT_BasePlayer.s().c_str(), hs::m_vecViewOffset.s().c_str());
	return *(Vector*)((DWORD)this + iOffset);
}

float& IBasePlayer::GetDuckSpeed()
{
	static auto m_flDuckSpeed = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_flDuckSpeed.s().c_str());
	return *(float*)((DWORD)this + m_flDuckSpeed);
}

const char* IBasePlayer::GetLastPlace() {
	return (const char*)((DWORD)this + 0x35B4);
}

float& IBasePlayer::GetDuckAmount()
{
	static int m_flDuckAmount = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_flDuckAmount.s().c_str());
	return *(float*)((DWORD)this + m_flDuckAmount);
}

void IBasePlayer::SetObserverMode(int value) {
	static int offset = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_iObserverMode.s().c_str());
	*reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(this) + offset) = value;
}

int32_t IBasePlayer::GetObserverMode()
{
	static int offset = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_iObserverMode.s().c_str());
	return *reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(this) + offset);
}

Vector IBasePlayer::GetEyeAngles()
{
	static int offset = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_angEyeAngles.s().c_str());
	return *(Vector*)((DWORD)this + offset);
}

float &IBasePlayer::GetVelocityModifier()
{
	static int offset = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_flVelocityModifier.s().c_str());
	return *(float*)((DWORD)this + offset);
}

Vector* IBasePlayer::GetEyeAnglesPointer()
{
	static int offset = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_angEyeAngles.s().c_str());
	return (Vector*)((DWORD)this + offset);
}

int IBasePlayer::GetSequence()
{
	static int offset = netvars.GetOffset(hs::DT_BaseAnimating.s().c_str(), hs::m_nSequence.s().c_str());
	return *(int*)((DWORD)this + offset);
}

float& IBasePlayer::GetFallVelocity()
{
	static int offset = netvars.GetOffset(hs::DT_BasePlayer.s().c_str(), hs::m_flFallVelocity.s().c_str());
	return *(float*)((DWORD)this + offset);
}
bool IBasePlayer::IsRagdoll() {
	return ((ClientClass*)this->GetClientClass())->m_ClassID == g_ClassID->CCSRagdoll;
}
bool IBasePlayer::IsPlayer()
{
	return ((ClientClass*)this->GetClientClass())->m_ClassID == g_ClassID->CCSPlayer;
}

bool IBasePlayer::DormantWrapped()
{
	if (!IsDormant())
		return false;

	return fabsf(csgo->LastSeenTime[GetIndex()] - csgo->get_absolute_time()) > 5.f;
}

int IBasePlayer::GetTeam()
{
	static int offset = netvars.GetOffset(hs::DT_BaseEntity.s().c_str(), hs::m_iTeamNum.s().c_str());
	return *(int*)((DWORD)this + offset);
}

float& IBasePlayer::GetFlashDuration()
{
	return *(float*)((DWORD)this + 0xA420);
}

bool IBasePlayer::IsFlashed() {
	return GetFlashDuration() > 0.f;
}

bool IBasePlayer::HaveDefuser() {
	static auto offset = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_bHasDefuser.s().c_str());
	return *(bool*)((DWORD)this + offset);
}

int IBasePlayer::GetArmor()
{
	static int offset = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_ArmorValue.s().c_str());
	return *(int*)((DWORD)this + offset);
}

bool IBaseCombatWeapon::m_bReloadVisuallyComplete() {
	static int offset = netvars.GetOffset(hs::DT_WeaponCSBase.s().c_str(), hs::m_bReloadVisuallyComplete.s().c_str());
	return *(bool*)((DWORD)this + offset);
}

int IBaseCombatWeapon::GetZoomLevel() {

	if (!this)
		return -1;
	static int offset = netvars.GetOffset(str("DT_WeaponCSBaseGun"), hs::m_zoomLevel.s().c_str());
	return *(int*)((DWORD)this + offset);
}

int IBaseCombatWeapon::GetAmmo(bool second)
{
	const auto data = GetCSWpnData();
	if (!this || !data)
		return 0;
	static int offset1 = netvars.GetOffset(hs::DT_BaseCombatWeapon.s().c_str(), hs::m_iClip1.s().c_str());
	if (second)
	{
		return data->m_iMaxClip1;
	}
	else
		return *(int*)((DWORD)this + offset1);
}

__forceinline float IBasePlayer::GetLastSeenTime() {
	static float time[65];
	float atime = csgo->get_absolute_time();
	if (!IsDormant()) {
		time[GetIndex()] = atime;
		return 0.f;
	}
	else
		return time[GetIndex()];
}

bool IBasePlayer::HasHelmet()
{
	static int m_ArmorValue = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_bHasHelmet.s().c_str());
	return *(bool*)(((DWORD)this + m_ArmorValue));
}

bool IBasePlayer::HeavyArmor()
{
	static int m_bHasHeavyArmor = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_bHasHeavyArmor.s().c_str());
	return *(bool*)(((DWORD)this + m_bHasHeavyArmor));
}

Vector IBasePlayer::GetAbsOriginVec()
{
	static unsigned int offset = FindInDataMap(GetPredDescMap(), hs::m_vecAbsOrigin.s().c_str());
	return *(Vector*)((DWORD)this + offset);
}

Vector IBasePlayer::GetAbsOrigin()
{
	return GetOrigin();
}

int IBasePlayer::Thrower()
{
	// NETVAR(thrower, "CBaseGrenade", "m_hThrower", int)
	static int m_hThrower = netvars.GetOffset(str("DT_BaseGrenade"), str("m_hThrower"));
	return *(int*)(((DWORD)this + m_hThrower));
}

float& IBasePlayer::GetSimulationTime()
{
	static int m_flSimulationTime = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_flSimulationTime.s().c_str());
	return *(float*)((DWORD)this + m_flSimulationTime);
}

int& IBasePlayer::GetEFlags()
{
	static unsigned int offset = FindInDataMap(GetPredDescMap(), hs::m_iEFlags.s().c_str());
	return *(int*)((uintptr_t)this + offset);
}

int& IBasePlayer::GetEffects() {
	static unsigned int offset = FindInDataMap(GetPredDescMap(), hs::m_fEffects.s().c_str());
	return *(int*)((uintptr_t)this + offset);
}

float IBaseCombatWeapon::LastShotTime() {
	static int m_fLastShotTime = netvars.GetOffset(hs::DT_WeaponCSBase.s().c_str(), hs::m_fLastShotTime.s().c_str());
	return *(float*)((DWORD)this + m_fLastShotTime);
}

float &IBaseCombatWeapon::GetRecoilIndex() {
	static int m_flRecoilIndex = netvars.GetOffset(hs::DT_WeaponCSBase.s().c_str(), hs::m_flRecoilIndex.s().c_str());
	return *(float*)((DWORD)this + m_flRecoilIndex);
}

float &IBaseCombatWeapon::GetAccuracyPenalty() {
	static int m_fAccuracyPenalty = netvars.GetOffset(hs::DT_WeaponCSBase.s().c_str(), hs::m_fAccuracyPenalty.s().c_str());
	return *(float*)((DWORD)this + m_fAccuracyPenalty);
}

bool IBaseCombatWeapon::StartedArming()
{
	static int offset = netvars.GetOffset(hs::DT_BaseCombatWeapon.s().c_str(), hs::m_bStartedArming.s().c_str());
	return *(bool*)((DWORD)this + offset);
}

Vector IBasePlayer::GetNetworkOrigin()
{
	static auto m_vecNetworkOrigin = FindInDataMap(GetPredDescMap(), str("m_vecNetworkOrigin"));
	return *reinterpret_cast<Vector*>(uintptr_t(this) + m_vecNetworkOrigin);
}

bool IBasePlayer::HasGunGameImmunity()
{
	if (!this)
		return false;
	static int offset = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_bGunGameImmunity.s().c_str());
	return *(bool*)((DWORD)this + offset);
}

float& IBasePlayer::GetMaxSpeed()
{
	static int m_flMaxSpeed = netvars.GetOffset(str("DT_BasePlayer"), str("m_flMaxSpeed"));
	return *(float*)(uintptr_t(this) + m_flMaxSpeed);
}

bool& IBasePlayer::IsDucking()
{
	static auto m_bDucking = netvars.GetOffset(str("DT_CSPlayer"), str("m_bDucking"));
	return *(bool*)(uintptr_t(this) + m_bDucking);
}

bool& IBasePlayer::IsWalking()
{
	static int m_bIsWalking = netvars.GetOffset(str("DT_CSPlayer"), str("m_bIsWalking"));
	return *(bool*)(uintptr_t(this) + m_bIsWalking);
}

int& IBasePlayer::GetMoveState()
{
	static int m_iMoveState = netvars.GetOffset(str("DT_CSPlayer"), str("m_iMoveState"));
	return *(int*)(uintptr_t(this) + m_iMoveState);
}

float& IBasePlayer::GetThirdpersonRecoil()
{
	static auto m_flThirdpersonRecoil = netvars.GetOffset(str("DT_CSPlayer"), str("m_flThirdpersonRecoil"));
	return *(float*)(uintptr_t(this) + m_flThirdpersonRecoil);
}

bool IBasePlayer::SetupBones(matrix* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
{
	void* pRenderable = reinterpret_cast<void*>(uintptr_t(this) + 0x4);
	if (!pRenderable)
		return false;

	typedef bool(__thiscall* Fn)(void*, matrix*, int, int, float);
	csgo->UpdateMatrix = true;
	auto res = getvfunc<Fn>(pRenderable, 13)(pRenderable, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
	csgo->UpdateMatrix = false;

	return res;
}

std::vector< IBaseCombatWeapon* > IBasePlayer::GetWeapons()
{
	int* m_hMyWeapons = reinterpret_cast< int* >((DWORD)this + 0x2DE8);
	std::vector< IBaseCombatWeapon* > list = {};
	for (auto i = 0; i < 64; ++i)
	{
		auto Weapon = interfaces.ent_list->GetClientEntityFromHandle(m_hMyWeapons[i]);
		if (Weapon)
		{
			list.push_back((IBaseCombatWeapon*)Weapon);
		}
	}
	return list;
}

int32_t IBaseCombatWeapon::WeaponMode()
{
	static int m_weaponMode = netvars.GetOffset(hs::DT_WeaponCSBaseGun.s().c_str(), hs::m_weaponMode.s().c_str());
	return *(int32_t*)((DWORD)this + m_weaponMode);
}

CCSWeaponInfo* IBaseCombatWeapon::GetCSWpnData()
{
	if (!this) return nullptr;
	typedef CCSWeaponInfo*(__thiscall* OriginalFn)(void*);
	return getvfunc<OriginalFn>(this, 460)(this);
}

int IBasePlayer::GetTickBase(void)
{
	static int m_nTickBase = netvars.GetOffset(hs::DT_BasePlayer.s().c_str(), hs::m_nTickBase.s().c_str());
	return *(int*)((DWORD)this + m_nTickBase);
}

int* IBasePlayer::GetImpulse()
{
	static std::uintptr_t m_nImpulse = FindInDataMap(GetPredDescMap(), hs::m_nImpulse.s().c_str());
	return (int*)((std::uintptr_t)this + m_nImpulse);
}

float& IBaseViewModel::GetAnimtime()
{
	static std::uintptr_t m_flAnimTime = FindInDataMap(GetPredDescMap(), hs::m_flAnimTime.s().c_str());
	return *(float*)((std::uintptr_t)this + m_flAnimTime);
}
float& IBasePlayer::GetAnimtime()
{
	static std::uintptr_t m_flAnimTime = FindInDataMap(GetPredDescMap(), hs::m_flAnimTime.s().c_str());
	return *(float*)((std::uintptr_t)this + m_flAnimTime);
}
float& IBasePlayer::GetCycle()
{
	static std::uintptr_t m_flCycle = FindInDataMap(GetPredDescMap(), hs::m_flCycle.s().c_str());
	return *(float*)((std::uintptr_t)this + m_flCycle);
}
float& IBaseViewModel::GetCycle()
{

	static std::uintptr_t m_flCycle = FindInDataMap(GetPredDescMap(), hs::m_flCycle.s().c_str());
	return *(float*)((std::uintptr_t)this + m_flCycle);
}

int &IBasePlayer::GetTickBasePtr(void)
{
	static int m_nTickBase = netvars.GetOffset(hs::DT_BasePlayer.s().c_str(), hs::m_nTickBase.s().c_str());
	return *(int*)((DWORD)this + m_nTickBase);
}

float& IBasePlayer::GetLBY()
{
	float f = 0.f;
	if (!this)
		return f;
	static int offset = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_flLowerBodyYawTarget.s().c_str());
	return *(float*)((DWORD)this + offset);
}

int IBasePlayer::GetHitboxSet()
{
	static int offset = netvars.GetOffset(hs::DT_BaseAnimating.s().c_str(), str("m_nHitboxSet"));
	return *(int*)((uintptr_t)this + offset);
}

std::string IBasePlayer::GetName()
{
	return GetPlayerInfo().name;
}

float IBaseCombatWeapon::GetPostponeFireReadyTime()
{
	static int offset = netvars.GetOffset(hs::DT_WeaponCSBase.s().c_str(), hs::m_flPostponeFireReadyTime.s().c_str());
	return *(float*)((uintptr_t)this + offset);
}

void IBaseCombatWeapon::SetPostPoneTime(float asdasdasd) {
	static int offset = netvars.GetOffset(hs::DT_WeaponCSBase.s().c_str(), hs::m_flPostponeFireReadyTime.s().c_str());
	*(float*)((uintptr_t)this + offset) = asdasdasd;
}

bool IBasePlayer::IsScoped()
{
	static int offset = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_bIsScoped.s().c_str());
	return *(bool*)((uintptr_t)this + offset);
}

int IBasePlayer::GetSequenceActivity(int sequence)
{
	auto model = this->GetModel();
	if (!model)
		return -1;
	auto hdr = interfaces.models.model_info->GetStudioModel(model);

	if (!hdr)
		return -1;

	// c_csplayer vfunc 242, follow calls to find the function.
	static DWORD fn = NULL;

	if (!fn) // 55 8B EC 83 7D 08 FF 56 8B F1 74
		fn = (DWORD)csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
			hs::seq_activity.s().c_str());

	static auto GetSequenceActivity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(fn);

	return GetSequenceActivity(this, hdr, sequence);
}

matrix& IBasePlayer::GetrgflCoordinateFrame()
{
	static int offset = netvars.GetOffset(hs::DT_BaseEntity.s().c_str(), hs::m_CollisionGroup.s().c_str()) - 48;
	return *(matrix*)(this + offset);
}

IBaseViewModel* IBasePlayer::GetViewModel()
{
	static int offset = netvars.GetOffset(hs::DT_BasePlayer.s().c_str(), hs::m_hViewModel.s().c_str());
	DWORD ViewModelData = *(DWORD*)((DWORD)this + offset);
	return (IBaseViewModel*)interfaces.ent_list->GetClientEntityFromHandle(ViewModelData);
}

CBaseHandle IBaseViewModel::GetViewmodelWeapon()
{
	static int offset = netvars.GetOffset(str("DT_BaseViewModel"), str("m_hWeapon"));
	return *(CBaseHandle*)(uintptr_t(this) + offset);
}

int IBaseViewModel::GetViewModelIndex()
{
	static int offset = netvars.GetOffset(str("DT_BaseViewModel"), str("m_nViewModelIndex"));
	return *(int*)(uintptr_t(this) + offset);
}

float& IBaseViewModel::GetCurrentCycle()
{
	static unsigned int m_flCycle = FindInDataMap(GetPredDescMap(), "m_flCycle");
	return *(float*)(uintptr_t(this) + m_flCycle);
}

float& IBaseViewModel::GetModelAnimTime()
{
	static unsigned int m_flAnimTime = FindInDataMap(GetPredDescMap(), "m_flAnimTime");
	return *(float*)(uintptr_t(this) + m_flAnimTime);
}

int& IBaseViewModel::GetCurrentSequence()
{
	static unsigned int m_nSequence = FindInDataMap(GetPredDescMap(), "m_nSequence");
	return *(int*)(uintptr_t(this) + m_nSequence);
}

int& IBaseViewModel::GetAnimationParity()
{
	static int m_nAnimationParity = netvars.GetOffset(str("DT_BaseViewModel"), str("m_nAnimationParity"));
	return *(int*)(uintptr_t(this) + m_nAnimationParity);
}

IBaseCombatWeapon* IBasePlayer::GetWeapon()
{
	static int offset = netvars.GetOffset(hs::DT_BasePlayer.s().c_str(), hs::m_hActiveWeapon.s().c_str());
	DWORD weaponData = *(DWORD*)((DWORD)this + offset);
	return (IBaseCombatWeapon*)interfaces.ent_list->GetClientEntityFromHandle(weaponData);
}

Vector IBasePlayer::GetPunchAngle()
{
	static int offset = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_aimPunchAngle.s().c_str());
	return *(Vector*)((DWORD)this + offset);
}
Vector& IBasePlayer::GetPunchAngleVel()
{
	static int offset = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_aimPunchAngleVel.s().c_str());
	return *(Vector*)((DWORD)this + offset);
}

Vector* IBasePlayer::GetPunchAnglePtr()
{
	static int offset = netvars.GetOffset(hs::DT_CSPlayer.s().c_str(), hs::m_aimPunchAngle.s().c_str());
	return (Vector*)((DWORD)this + offset);
}

Vector IBasePlayer::GetViewPunchAngle()
{
	static int offset = netvars.GetOffset(hs::DT_BasePlayer.s().c_str(), hs::m_viewPunchAngle.s().c_str());
	return *(Vector*)((DWORD)this + offset);
}

Vector* IBasePlayer::GetViewPunchAnglePtr()
{
	static int offset = netvars.GetOffset(hs::DT_BasePlayer.s().c_str(), hs::m_viewPunchAngle.s().c_str());
	return (Vector*)((DWORD)this + offset);
}

Vector& IBasePlayer::GetAbsAngles()
{
	typedef Vector& (__thiscall* Fn)(void*);
	return (getvfunc<Fn>(this, 11))(this);
}

Vector& IBasePlayer::GetAbsOriginVirtual()
{
	typedef Vector& (__thiscall* Fn)(void*);
	return (getvfunc<Fn>(this, 10))(this);
}

bool& IBasePlayer::GetClientSideAnims()
{
	static int offset = netvars.GetOffset(hs::DT_BaseAnimating.s().c_str(), hs::m_bClientSideAnimation.s().c_str());
	return *reinterpret_cast<bool*>(uintptr_t(this) + offset);
}

float IBasePlayer::GetDSYDelta()
{
	auto animstate = this->GetPlayerAnimState();
	auto speedfraction = max(0.f, min(animstate->m_speed_as_portion_of_walk_top_speed, 1.f));
	auto speedfactor = max(0.f, min(animstate->m_speed_as_portion_of_crouch_top_speed, 1.f));

	auto lol = ((animstate->m_walk_run_transition * -0.30000001f) - 0.19999999f) * speedfraction + 1.f;

	if (animstate->m_anim_duck_amount > 0.0f)
		lol += ((animstate->m_anim_duck_amount * speedfactor) * (0.5f - lol));

	return (animstate->m_aim_yaw_max * lol);
}

CUtlVector< CAnimationLayer >& IBasePlayer::GetAnimOverlayVector() {
	static auto AnimOverlayVector = *(DWORD*)(csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		str("8B 89 ?? ?? ?? ?? 8D 0C D1")) + 2);
	return *(CUtlVector< CAnimationLayer >*)((uintptr_t)this + AnimOverlayVector);
}

CCSGOPlayerAnimState* IBasePlayer::GetPlayerAnimState()
{
	if (!this->isAlive())
		return nullptr;
	return *reinterpret_cast<CCSGOPlayerAnimState**>(uintptr_t(this) + 0x3914);
}

void IBasePlayer::InvalidatePhysicsRecursive(int32_t flags) {
	static const auto invalidate_physics_recursive = reinterpret_cast<void(__thiscall*)(IBasePlayer*, int32_t)>(
		csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()), hs::physics_recursive.s().c_str()));
	invalidate_physics_recursive(this, flags);
}

void IBasePlayer::SetAnimState(CCSGOPlayerAnimState* anims) {
	*reinterpret_cast<CCSGOPlayerAnimState**>(uintptr_t(this) + 0x3914) = anims;
}

Vector& IBasePlayer::GetVecForce()
{
	static auto offset = netvars.GetOffset(str("DT_CSRagdoll"), str("m_vecForce"));
	return *(Vector*)((uintptr_t)this + offset);
}

short& IBaseCombatWeapon::GetItemDefinitionIndex()
{
	static int offset = netvars.GetOffset(hs::DT_BaseAttributableItem.s().c_str(), hs::m_iItemDefinitionIndex.s().c_str());
	return *(short*)((DWORD)this + offset);
}
bool IBaseCombatWeapon::IsKnife()
{
	//int iWeaponID = this->GetItemDefinitionIndex();
	//return (iWeaponID == tknife || iWeaponID == ctknife
	//	|| iWeaponID == goldknife || iWeaponID == 59 || iWeaponID == 41
	//	|| iWeaponID == 500 || iWeaponID == 505 || iWeaponID == 506
	//	|| iWeaponID == 507 || iWeaponID == 508 || iWeaponID == 509
	//	|| iWeaponID == 515);
	const auto& classId = ((IBasePlayer*)this)->GetClientClass();
	if (!classId)
		return false;
	return classId->m_ClassID == g_ClassID->CKnife || classId->m_ClassID == g_ClassID->CKnifeGG;
}
bool IBaseCombatWeapon::IsNade()
{
	const auto& classId = ((IBasePlayer*)this)->GetClientClass();
	if (!classId)
		return false;
	if (classId->m_ClassID == g_ClassID->CSnowball)
		return true;
	int WeaponId = this->GetItemDefinitionIndex();

	return WeaponId == WEAPON_FLASHBANG || WeaponId == WEAPON_HEGRENADE || WeaponId == WEAPON_SMOKEGRENADE ||
		WeaponId == WEAPON_MOLOTOV || WeaponId == WEAPON_DECOY || WeaponId == WEAPON_INC;
}
float& IBasePlayer::m_flNextAttack()
{
	return *(float*)((uintptr_t)this + 0x2D70);
}
int IBaseCombatWeapon::Owner() {
	//  NETVAR(ownerEntity, "CBaseEntity", "m_hOwnerEntity", int)

	static int offset = netvars.GetOffset(hs::DT_BaseEntity.s().c_str(), str("m_hOwnerEntity"));
	return *(int*)((DWORD)this + offset);
}

bool IBaseCombatWeapon::InReload()
{
	return *(bool*)(this + 0x32A5);
}

float& IBaseCombatWeapon::NextSecondaryAttack()
{
	static int offset = netvars.GetOffset(hs::DT_BaseCombatWeapon.s().c_str(), hs::m_flNextSecondaryAttack.s().c_str());
	return *(float*)((DWORD)this + offset);
}

float IBaseCombatWeapon::NextPrimaryAttack()
{
	static int offset = netvars.GetOffset(hs::DT_BaseCombatWeapon.s().c_str(), hs::m_flNextPrimaryAttack.s().c_str());
	return *(float*)((DWORD)this + offset);
}

float IBaseCombatWeapon::GetLastShotTime()
{
	static int offset = netvars.GetOffset(hs::DT_WeaponCSBase.s().c_str(), hs::m_fLastShotTime.s().c_str());
	return *(float*)((DWORD)this + offset);
}

float CBaseCSGrenade::GetThrowTime()
{
	static int m_fThrowTime = netvars.GetOffset((hs::DT_BaseCSGrenade.s().c_str()), hs::m_fThrowTime.s().c_str());
	return *(float*)((uintptr_t)this + m_fThrowTime);
}

bool IBaseCombatWeapon::CanFire()
{

	float nextPrimAttack = csgo->weapon->NextPrimaryAttack();
	float serverTime = TICKS_TO_TIME(csgo->local->GetTickBase());// + I::pGlobals->frametime;absoluteframetime

	if (csgo->weapon->GetAmmo(false) < 1 || csgo->weapon->InReload())
		return false;

	/*if (cfg.Rage.AutoFire.autoRevolver)
	{
	if (!aimbotV2->IsRevolverAbleToShoot())
	return false;
	}*/

	if (/*nextPrimAttack <= 0 || */nextPrimAttack < serverTime)
		return true;

}

#pragma region meme

unsigned int FindInDataMap(datamap_t *pMap, const char *name)
{
	while (pMap)
	{
		for (int i = 0; i<pMap->dataNumFields; i++)
		{
			if (pMap->dataDesc[i].fieldName == NULL)
				continue;

			if (strcmp(name, pMap->dataDesc[i].fieldName) == 0)
				return pMap->dataDesc[i].fieldOffset[TD_OFFSET_NORMAL];

			if (pMap->dataDesc[i].fieldType == 10)
			{
				if (pMap->dataDesc[i].td)
				{
					unsigned int offset;

					if ((offset = FindInDataMap(pMap->dataDesc[i].td, name)) != 0)
						return offset;
				}
			}
		}
		pMap = pMap->baseMap;
	}

	return 0;
}
#pragma endregion

ClassIdManager * g_ClassID = nullptr;

inline int ClassIdManager::GetClassID(const char * classname)
{
	ServerClass * serverclass = interfaces.server->GetAllServerClasses();
	int id = 0;
	while (serverclass)
	{
		if (!strcmp(serverclass->m_pNetworkName, classname))
			return id;
		serverclass = serverclass->m_pNext, id++;
	}
	return -1;
}

ClassIdManager::ClassIdManager()
{
	CAI_BaseNPC = GetClassID(str("CAI_BaseNPC"));
	CAK47 = GetClassID(str("CAK47"));
	CBaseAnimating = GetClassID(str("CBaseAnimating"));
	CBaseAnimatingOverlay = GetClassID(str("CBaseAnimatingOverlay"));
	CBaseAttributableItem = GetClassID(str("CBaseAttributableItem"));
	CBaseButton = GetClassID(str("CBaseButton"));
	CBaseCombatCharacter = GetClassID(str("CBaseCombatCharacter"));
	CBaseCombatWeapon = GetClassID(str("CBaseCombatWeapon"));
	CBaseCSGrenade = GetClassID(str("CBaseCSGrenade"));
	CBaseCSGrenadeProjectile = GetClassID(str("CBaseCSGrenadeProjectile"));
	CBaseDoor = GetClassID(str("CBaseDoor"));
	CBaseEntity = GetClassID(str("CBaseEntity"));
	CBaseFlex = GetClassID(str("CBaseFlex"));
	CBaseGrenade = GetClassID(str("CBaseGrenade"));
	CBaseParticleEntity = GetClassID(str("CBaseParticleEntity"));
	CBasePlayer = GetClassID(str("CBasePlayer"));
	CBasePropDoor = GetClassID(str("CBasePropDoor"));
	CBaseTeamObjectiveResource = GetClassID(str("CBaseTeamObjectiveResource"));
	CBaseTempEntity = GetClassID(str("CBaseTempEntity"));
	CBaseToggle = GetClassID(str("CBaseToggle"));
	CBaseTrigger = GetClassID(str("CBaseTrigger"));
	CBaseViewModel = GetClassID(str("CBaseViewModel"));
	CBaseVPhysicsTrigger = GetClassID(str("CBaseVPhysicsTrigger"));
	CBaseWeaponWorldModel = GetClassID(str("CBaseWeaponWorldModel"));
	CBeam = GetClassID(str("CBeam"));
	CBeamSpotlight = GetClassID(str("CBeamSpotlight"));
	CBoneFollower = GetClassID(str("CBoneFollower"));
	CBRC4Target = GetClassID(str("CBRC4Target"));
	CBreachCharge = GetClassID(str("CBreachCharge"));
	CBreachChargeProjectile = GetClassID(str("CBreachChargeProjectile"));
	CBreakableProp = GetClassID(str("CBreakableProp"));
	CBreakableSurface = GetClassID(str("CBreakableSurface"));
	CC4 = GetClassID(str("CC4"));
	CCascadeLight = GetClassID(str("CCascadeLight"));
	CChicken = GetClassID(str("CChicken"));
	CColorCorrection = GetClassID(str("CColorCorrection"));
	CColorCorrectionVolume = GetClassID(str("CColorCorrectionVolume"));
	CCSGameRulesProxy = GetClassID(str("CCSGameRulesProxy"));
	CCSPlayer = GetClassID(hs::CCSPlayer.s().c_str());
	CCSPlayerResource = GetClassID(str("CCSPlayerResource"));
	CCSRagdoll = GetClassID(str("CCSRagdoll"));
	CCSTeam = GetClassID(str("CCSTeam"));
	CDangerZone = GetClassID(str("CDangerZone"));
	CDangerZoneController = GetClassID(str("CDangerZoneController"));
	CDEagle = GetClassID(str("CDEagle"));
	CDecoyGrenade = GetClassID(str("CDecoyGrenade"));
	CDecoyProjectile = GetClassID(str("CDecoyProjectile"));
	CDrone = GetClassID(str("CDrone"));
	CDronegun = GetClassID(str("CDronegun"));
	CDynamicLight = GetClassID(str("CDynamicLight"));
	CDynamicProp = GetClassID(str("CDynamicProp"));
	CEconEntity = GetClassID(str("CEconEntity"));
	CEconWearable = GetClassID(str("CEconWearable"));
	CEmbers = GetClassID(str("CEmbers"));
	CEntityDissolve = GetClassID(str("CEntityDissolve"));
	CEntityFlame = GetClassID(str("CEntityFlame"));
	CEntityFreezing = GetClassID(str("CEntityFreezing"));
	CEntityParticleTrail = GetClassID(str("CEntityParticleTrail"));
	CEnvAmbientLight = GetClassID(str("CEnvAmbientLight"));
	CEnvDetailController = GetClassID(str("CEnvDetailController"));
	CEnvDOFController = GetClassID(str("CEnvDOFController"));
	CEnvGasCanister = GetClassID(str("CEnvGasCanister"));
	CEnvParticleScript = GetClassID(str("CEnvParticleScript"));
	CEnvProjectedTexture = GetClassID(str("CEnvProjectedTexture"));
	CEnvQuadraticBeam = GetClassID(str("CEnvQuadraticBeam"));
	CEnvScreenEffect = GetClassID(str("CEnvScreenEffect"));
	CEnvScreenOverlay = GetClassID(str("CEnvScreenOverlay"));
	CEnvTonemapController = GetClassID(str("CEnvTonemapController"));
	CEnvWind = GetClassID(str("CEnvWind"));
	CFEPlayerDecal = GetClassID(str("CFEPlayerDecal"));
	CFireCrackerBlast = GetClassID(str("CFireCrackerBlast"));
	CFireSmoke = GetClassID(str("CFireSmoke"));
	CFireTrail = GetClassID(str("CFireTrail"));
	CFish = GetClassID(str("CFish"));
	CFists = GetClassID(str("CFists"));
	CFlashbang = GetClassID(str("CFlashbang"));
	CFogController = GetClassID(str("CFogController"));
	CFootstepControl = GetClassID(str("CFootstepControl"));
	CFunc_Dust = GetClassID(str("CFunc_Dust"));
	CFunc_LOD = GetClassID(str("CFunc_LOD"));
	CFuncAreaPortalWindow = GetClassID(str("CFuncAreaPortalWindow"));
	CFuncBrush = GetClassID(str("CFuncBrush"));
	CFuncConveyor = GetClassID(str("CFuncConveyor"));
	CFuncLadder = GetClassID(str("CFuncLadder"));
	CFuncMonitor = GetClassID(str("CFuncMonitor"));
	CFuncMoveLinear = GetClassID(str("CFuncMoveLinear"));
	CFuncOccluder = GetClassID(str("CFuncOccluder"));
	CFuncReflectiveGlass = GetClassID(str("CFuncReflectiveGlass"));
	CFuncRotating = GetClassID(str("CFuncRotating"));
	CFuncSmokeVolume = GetClassID(str("CFuncSmokeVolume"));
	CFuncTrackTrain = GetClassID(str("CFuncTrackTrain"));
	CGameRulesProxy = GetClassID(str("CGameRulesProxy"));
	CGrassBurn = GetClassID(str("CGrassBurn"));
	CHandleTest = GetClassID(str("CHandleTest"));
	CHEGrenade = GetClassID(str("CHEGrenade"));
	CHostage = GetClassID(str("CHostage"));
	CHostageCarriableProp = GetClassID(str("CHostageCarriableProp"));
	CIncendiaryGrenade = GetClassID(str("CIncendiaryGrenade"));
	CInferno = GetClassID(str("CInferno"));
	CInfoLadderDismount = GetClassID(str("CInfoLadderDismount"));
	CInfoMapRegion = GetClassID(str("CInfoMapRegion"));
	CInfoOverlayAccessor = GetClassID(str("CInfoOverlayAccessor"));
	CItem_Healthshot = GetClassID(str("CItem_Healthshot"));
	CItemCash = GetClassID(str("CItemCash"));
	CItemDogtags = GetClassID(str("CItemDogtags"));
	CKnife = GetClassID(str("CKnife"));
	CKnifeGG = GetClassID(str("CKnifeGG"));
	CLightGlow = GetClassID(str("CLightGlow"));
	CMaterialModifyControl = GetClassID(str("CMaterialModifyControl"));
	CMelee = GetClassID(str("CMelee"));
	CMolotovGrenade = GetClassID(str("CMolotovGrenade"));
	CMolotovProjectile = GetClassID(str("CMolotovProjectile"));
	CMovieDisplay = GetClassID(str("CMovieDisplay"));
	CParadropChopper = GetClassID(str("CParadropChopper"));
	CParticleFire = GetClassID(str("CParticleFire"));
	CParticlePerformanceMonitor = GetClassID(str("CParticlePerformanceMonitor"));
	CParticleSystem = GetClassID(str("CParticleSystem"));
	CPhysBox = GetClassID(str("CPhysBox"));
	CPhysBoxMultiplayer = GetClassID(str("CPhysBoxMultiplayer"));
	CPhysicsProp = GetClassID(str("CPhysicsProp"));
	CPhysicsPropMultiplayer = GetClassID(str("CPhysicsPropMultiplayer"));
	CPhysMagnet = GetClassID(str("CPhysMagnet"));
	CPhysPropAmmoBox = GetClassID(str("CPhysPropAmmoBox"));
	CPhysPropLootCrate = GetClassID(str("CPhysPropLootCrate"));
	CPhysPropRadarJammer = GetClassID(str("CPhysPropRadarJammer"));
	CPhysPropWeaponUpgrade = GetClassID(str("CPhysPropWeaponUpgrade"));
	CPlantedC4 = GetClassID(str("CPlantedC4"));
	CPlasma = GetClassID(str("CPlasma"));
	CPlayerResource = GetClassID(str("CPlayerResource"));
	CPointCamera = GetClassID(str("CPointCamera"));
	CPointCommentaryNode = GetClassID(str("CPointCommentaryNode"));
	CPointWorldText = GetClassID(str("CPointWorldText"));
	CPoseController = GetClassID(str("CPoseController"));
	CPostProcessController = GetClassID(str("CPostProcessController"));
	CPrecipitation = GetClassID(str("CPrecipitation"));
	CPrecipitationBlocker = GetClassID(str("CPrecipitationBlocker"));
	CPredictedViewModel = GetClassID(str("CPredictedViewModel"));
	CProp_Hallucination = GetClassID(str("CProp_Hallucination"));
	CPropCounter = GetClassID(str("CPropCounter"));
	CPropDoorRotating = GetClassID(str("CPropDoorRotating"));
	CPropJeep = GetClassID(str("CPropJeep"));
	CPropVehicleDriveable = GetClassID(str("CPropVehicleDriveable"));
	CRagdollManager = GetClassID(str("CRagdollManager"));
	CRagdollProp = GetClassID(str("CRagdollProp"));
	CRagdollPropAttached = GetClassID(str("CRagdollPropAttached"));
	CRopeKeyframe = GetClassID(str("CRopeKeyframe"));
	CSCAR17 = GetClassID(str("CSCAR17"));
	CSceneEntity = GetClassID(str("CSceneEntity"));
	CSensorGrenade = GetClassID(str("CSensorGrenade"));
	CSensorGrenadeProjectile = GetClassID(str("CSensorGrenadeProjectile"));
	CShadowControl = GetClassID(str("CShadowControl"));
	CSlideshowDisplay = GetClassID(str("CSlideshowDisplay"));
	CSmokeGrenade = GetClassID(str("CSmokeGrenade"));
	CSmokeGrenadeProjectile = GetClassID(str("CSmokeGrenadeProjectile"));
	CSmokeStack = GetClassID(str("CSmokeStack"));
	CSnowball = GetClassID(str("CSnowball"));
	CSnowballPile = GetClassID(str("CSnowballPile"));
	CSnowballProjectile = GetClassID(str("CSnowballProjectile"));
	CSpatialEntity = GetClassID(str("CSpatialEntity"));
	CSpotlightEnd = GetClassID(str("CSpotlightEnd"));
	CSprite = GetClassID(str("CSprite"));
	CSpriteOriented = GetClassID(str("CSpriteOriented"));
	CSpriteTrail = GetClassID(str("CSpriteTrail"));
	CStatueProp = GetClassID(str("CStatueProp"));
	CSteamJet = GetClassID(str("CSteamJet"));
	CSun = GetClassID(str("CSun"));
	CSunlightShadowControl = GetClassID(str("CSunlightShadowControl"));
	CSurvivalSpawnChopper = GetClassID(str("CSurvivalSpawnChopper"));
	CTablet = GetClassID(str("CTablet"));
	CTeam = GetClassID(str("CTeam"));
	CTeamplayRoundBasedRulesProxy = GetClassID(str("CTeamplayRoundBasedRulesProxy"));
	CTEArmorRicochet = GetClassID(str("CTEArmorRicochet"));
	CTEBaseBeam = GetClassID(str("CTEBaseBeam"));
	CTEBeamEntPoint = GetClassID(str("CTEBeamEntPoint"));
	CTEBeamEnts = GetClassID(str("CTEBeamEnts"));
	CTEBeamFollow = GetClassID(str("CTEBeamFollow"));
	CTEBeamLaser = GetClassID(str("CTEBeamLaser"));
	CTEBeamPoints = GetClassID(str("CTEBeamPoints"));
	CTEBeamRing = GetClassID(str("CTEBeamRing"));
	CTEBeamRingPoint = GetClassID(str("CTEBeamRingPoint"));
	CTEBeamSpline = GetClassID(str("CTEBeamSpline"));
	CTEBloodSprite = GetClassID(str("CTEBloodSprite"));
	CTEBloodStream = GetClassID(str("CTEBloodStream"));
	CTEBreakModel = GetClassID(str("CTEBreakModel"));
	CTEBSPDecal = GetClassID(str("CTEBSPDecal"));
	CTEBubbles = GetClassID(str("CTEBubbles"));
	CTEBubbleTrail = GetClassID(str("CTEBubbleTrail"));
	CTEClientProjectile = GetClassID(str("CTEClientProjectile"));
	CTEDecal = GetClassID(str("CTEDecal"));
	CTEDust = GetClassID(str("CTEDust"));
	CTEDynamicLight = GetClassID(str("CTEDynamicLight"));
	CTEEffectDispatch = GetClassID(str("CTEEffectDispatch"));
	CTEEnergySplash = GetClassID(str("CTEEnergySplash"));
	CTEExplosion = GetClassID(str("CTEExplosion"));
	CTEFireBullets = GetClassID(str("CTEFireBullets"));
	CTEFizz = GetClassID(str("CTEFizz"));
	CTEFootprintDecal = GetClassID(str("CTEFootprintDecal"));
	CTEFoundryHelpers = GetClassID(str("CTEFoundryHelpers"));
	CTEGaussExplosion = GetClassID(str("CTEGaussExplosion"));
	CTEGlowSprite = GetClassID(str("CTEGlowSprite"));
	CTEImpact = GetClassID(str("CTEImpact"));
	CTEKillPlayerAttachments = GetClassID(str("CTEKillPlayerAttachments"));
	CTELargeFunnel = GetClassID(str("CTELargeFunnel"));
	CTEMetalSparks = GetClassID(str("CTEMetalSparks"));
	CTEMuzzleFlash = GetClassID(str("CTEMuzzleFlash"));
	CTEParticleSystem = GetClassID(str("CTEParticleSystem"));
	CTEPhysicsProp = GetClassID(str("CTEPhysicsProp"));
	CTEPlantBomb = GetClassID(str("CTEPlantBomb"));
	CTEPlayerAnimEvent = GetClassID(str("CTEPlayerAnimEvent"));
	CTEPlayerDecal = GetClassID(str("CTEPlayerDecal"));
	CTEProjectedDecal = GetClassID(str("CTEProjectedDecal"));
	CTERadioIcon = GetClassID(str("CTERadioIcon"));
	CTEShatterSurface = GetClassID(str("CTEShatterSurface"));
	CTEShowLine = GetClassID(str("CTEShowLine"));
	CTesla = GetClassID(str("CTesla"));
	CTESmoke = GetClassID(str("CTESmoke"));
	CTESparks = GetClassID(str("CTESparks"));
	CTESprite = GetClassID(str("CTESprite"));
	CTESpriteSpray = GetClassID(str("CTESpriteSpray"));
	CTest_ProxyToggle_Networkable = GetClassID(str("CTest_ProxyToggle_Networkable"));
	CTestTraceline = GetClassID(str("CTestTraceline"));
	CTEWorldDecal = GetClassID(str("CTEWorldDecal"));
	CTriggerPlayerMovement = GetClassID(str("CTriggerPlayerMovement"));
	CTriggerSoundOperator = GetClassID(str("CTriggerSoundOperator"));
	CVGuiScreen = GetClassID(str("CVGuiScreen"));
	CVoteController = GetClassID(str("CVoteController"));
	CWaterBullet = GetClassID(str("CWaterBullet"));
	CWaterLODControl = GetClassID(str("CWaterLODControl"));
	CWeaponAug = GetClassID(str("CWeaponAug"));
	CWeaponAWP = GetClassID(str("CWeaponAWP"));
	CWeaponBaseItem = GetClassID(str("CWeaponBaseItem"));
	CWeaponBizon = GetClassID(str("CWeaponBizon"));
	CWeaponCSBase = GetClassID(str("CWeaponCSBase"));
	CWeaponCSBaseGun = GetClassID(str("CWeaponCSBaseGun"));
	CWeaponCycler = GetClassID(str("CWeaponCycler"));
	CWeaponElite = GetClassID(str("CWeaponElite"));
	CWeaponFamas = GetClassID(str("CWeaponFamas"));
	CWeaponFiveSeven = GetClassID(str("CWeaponFiveSeven"));
	CWeaponG3SG1 = GetClassID(str("CWeaponG3SG1"));
	CWeaponGalil = GetClassID(str("CWeaponGalil"));
	CWeaponGalilAR = GetClassID(str("CWeaponGalilAR"));
	CWeaponGlock = GetClassID(str("CWeaponGlock"));
	CWeaponHKP2000 = GetClassID(str("CWeaponHKP2000"));
	CWeaponM249 = GetClassID(str("CWeaponM249"));
	CWeaponM3 = GetClassID(str("CWeaponM3"));
	CWeaponM4A1 = GetClassID(str("CWeaponM4A1"));
	CWeaponMAC10 = GetClassID(str("CWeaponMAC10"));
	CWeaponMag7 = GetClassID(str("CWeaponMag7"));
	CWeaponMP5Navy = GetClassID(str("CWeaponMP5Navy"));
	CWeaponMP7 = GetClassID(str("CWeaponMP7"));
	CWeaponMP9 = GetClassID(str("CWeaponMP9"));
	CWeaponNegev = GetClassID(str("CWeaponNegev"));
	CWeaponNOVA = GetClassID(str("CWeaponNOVA"));
	CWeaponP228 = GetClassID(str("CWeaponP228"));
	CWeaponP250 = GetClassID(str("CWeaponP250"));
	CWeaponP90 = GetClassID(str("CWeaponP90"));
	CWeaponSawedoff = GetClassID(str("CWeaponSawedoff"));
	CWeaponSCAR20 = GetClassID(str("CWeaponSCAR20"));
	CWeaponScout = GetClassID(str("CWeaponScout"));
	CWeaponSG550 = GetClassID(str("CWeaponSG550"));
	CWeaponSG552 = GetClassID(str("CWeaponSG552"));
	CWeaponSG556 = GetClassID(str("CWeaponSG556"));
	CWeaponSSG08 = GetClassID(str("CWeaponSSG08"));
	CWeaponTaser = GetClassID(str("CWeaponTaser"));
	CWeaponTec9 = GetClassID(str("CWeaponTec9"));
	CWeaponTMP = GetClassID(str("CWeaponTMP"));
	CWeaponUMP45 = GetClassID(str("CWeaponUMP45"));
	CWeaponUSP = GetClassID(str("CWeaponUSP"));
	CWeaponXM1014 = GetClassID(str("CWeaponXM1014"));
	CWorld = GetClassID(str("CWorld"));
	CWorldVguiText = GetClassID(str("CWorldVguiText"));
	DustTrail = GetClassID(str("DustTrail"));
	MovieExplosion = GetClassID(str("MovieExplosion"));
	ParticleSmokeGrenade = GetClassID(str("ParticleSmokeGrenade"));
	RocketTrail = GetClassID(str("RocketTrail"));
	SmokeTrail = GetClassID(str("SmokeTrail"));
	SporeExplosion = GetClassID(str("SporeExplosion"));
	SporeTrail = GetClassID(str("SporeTrail"));
}