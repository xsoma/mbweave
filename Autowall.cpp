#include "Hooks.h"
#include "Autowall.h"

ReturnInfo_t CAutoWall::Think(Vector pos, IBasePlayer* target, int specific_hitgroup, Vector eye_pos, IBasePlayer* start_ent)
{
	ReturnInfo_t return_info = ReturnInfo_t(-1, -1, 4, false, 0.f, nullptr);

	if (!start_ent)
		return return_info;

	auto cached_pos = pos;
	for (int i = 0; i < scanned_points.size(); i++) {
		if (cached_pos == scanned_points[i]) {
			return_info.m_damage = scanned_damage[i];
			return return_info;
		}
	}

	Vector start = eye_pos;

	FireBulletData_t fire_bullet_data;
	fire_bullet_data.m_start = start;
	fire_bullet_data.m_end = pos;
	fire_bullet_data.m_current_position = start;
	fire_bullet_data.m_penetration_count = 4;

	Math::AngleVectors(Math::CalculateAngle(start, pos), fire_bullet_data.m_direction);

	static const auto filter_simple = *reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(
		(void*)csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
			hs::trace_filter.s().c_str())) + 0x3d);

	uint32_t dwFilter[4] = { filter_simple,
		reinterpret_cast<uint32_t>(start_ent), 0, 0 };

	fire_bullet_data.m_filter = (CTraceFilter*)(dwFilter);

	auto weapon = start_ent->GetWeapon();
	if (!weapon)
		return return_info;

	auto weapon_info = weapon->GetCSWpnData();
	if (!weapon_info)
		return return_info;

	float range = min(weapon_info->m_flRange, (start - pos).Length());

	pos = start + (fire_bullet_data.m_direction * range);
	fire_bullet_data.m_current_damage = weapon_info->m_iDamage;

	while (fire_bullet_data.m_current_damage > 0 && fire_bullet_data.m_penetration_count > 0)
	{
		return_info.m_penetration_count = fire_bullet_data.m_penetration_count;

		TraceLine(fire_bullet_data.m_current_position, pos, MASK_SHOT | CONTENTS_GRATE, start_ent, &fire_bullet_data.m_enter_trace);
		{
			Vector end = fire_bullet_data.m_current_position + (fire_bullet_data.m_direction * 40.f);
			ClipTrace(fire_bullet_data.m_current_position, end, target, MASK_SHOT | CONTENTS_GRATE, fire_bullet_data.m_filter, &fire_bullet_data.m_enter_trace);
		}
		const float distance_traced = (fire_bullet_data.m_enter_trace.endpos - start).Length();

		fire_bullet_data.m_current_damage *= pow(weapon_info->m_flRangeModifier, (distance_traced / 500.f));
		
		if (fire_bullet_data.m_enter_trace.fraction == 1.f)
		{
			return_info.m_damage = fire_bullet_data.m_current_damage;
			return_info.m_hitgroup = fire_bullet_data.m_enter_trace.hitgroup;
			return_info.m_end = fire_bullet_data.m_enter_trace.endpos;
			return_info.m_hit_entity = nullptr;
			break;
		}

		if (fire_bullet_data.m_enter_trace.hitgroup > 0 && fire_bullet_data.m_enter_trace.hitgroup <= 8)
		{
			if (
				(fire_bullet_data.m_enter_trace.hitgroup != specific_hitgroup)
				||
				(fire_bullet_data.m_enter_trace.m_pEnt != target)
				||
				((fire_bullet_data.m_enter_trace.m_pEnt)->GetTeam() == start_ent->GetTeam()))
			{
				return_info.m_damage = -1;
				return return_info;
			}

			ScaleDamage(fire_bullet_data.m_enter_trace.m_pEnt, weapon_info, fire_bullet_data.m_enter_trace.hitgroup, fire_bullet_data.m_current_damage);

			return_info.m_damage = fire_bullet_data.m_current_damage;
			return_info.m_hitgroup = fire_bullet_data.m_enter_trace.hitgroup;
			return_info.m_end = fire_bullet_data.m_enter_trace.endpos;
			return_info.m_hit_entity = fire_bullet_data.m_enter_trace.m_pEnt;

			break;
		}

		if (!HandleBulletPenetration(weapon_info, fire_bullet_data))
			break;

		return_info.m_did_penetrate_wall = true;
	}

	scanned_damage.push_back(return_info.m_damage);
	scanned_points.push_back(cached_pos);

	return_info.m_penetration_count = fire_bullet_data.m_penetration_count;

	return return_info;
}

float CAutoWall::HitgroupDamage(int iHitGroup)
{
	switch (iHitGroup)
	{
	case HITGROUP_HEAD:
		return 4.f;
		break;
	case HITGROUP_CHEST:
	case 8:
		return 1.f;
		break;
	case HITGROUP_STOMACH:
		return 1.25f;
		break;
	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:
		return 1.f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		return 0.75f;
		break;
	default:
		break;
	}
	return 1.f;
}

bool CAutoWall::IsArmored(IBasePlayer* player, int hitgroup)
{
	if (!player)
		return false;
	if (player->GetArmor() > 0)
	{
		switch (hitgroup)
		{
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
		case 8:
			return true;
			break;
		case HITGROUP_HEAD:
			return player->HasHelmet() || (bool)player->HeavyArmor();
			break;
		default:
			return (bool)player->HeavyArmor();
			break;
		}
	}

	return false;
}

void CAutoWall::ScaleDamage(IBasePlayer* player, CCSWeaponInfo* weapon_info, int hitgroup, float& damage)
{
	if (!player)
		return;

	auto new_damage = damage;

	const auto is_zeus = csgo->weapon->GetItemDefinitionIndex() == weapon_taser;

	static auto is_armored = [](IBasePlayer* player, int armor, int hitgroup) {
		if (player && player->GetArmor() > 0)
		{
			if (player->HasHelmet() && hitgroup == HITGROUP_HEAD || (hitgroup >= HITGROUP_CHEST && hitgroup <= HITGROUP_RIGHTARM))
				return true;
		}
		return false;
	};

	if (!is_zeus) {
		switch (hitgroup)
		{
		case HITGROUP_HEAD:
			new_damage *= 4.f;
			break;
		case HITGROUP_STOMACH:
			new_damage *= 1.25f;
			break;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
			new_damage *= .75f;
			break;
		default:
			break;
			/*4.0; 1
			1.0; 2
			1.25; 3
			1.0; 4
			1.0; 5
			0.75; 6
			0.75; 7
			1.0; 8*/
		}
	}
	else
		new_damage *= 0.92f;

	auto weaponData = csgo->weapon->GetCSWpnData();
	if (!weaponData)
		return;

	if (is_armored(player, player->GetArmor(), hitgroup))
	{
		float flHeavyRatio = 1.0f;
		float flBonusRatio = 0.5f;
		float flRatio = weaponData->m_flArmorRatio * 0.5f;
		float flNewDamage;

		if (!player->HeavyArmor())
		{
			flNewDamage = new_damage * flRatio;
		}
		else
		{
			flBonusRatio = 0.33f;
			flRatio = weaponData->m_flArmorRatio * 0.5f;
			flHeavyRatio = 0.33f;
			flNewDamage = (new_damage * (flRatio * 0.5)) * 0.85f;
		}

		int iArmor = player->GetArmor();

		if (((new_damage - flNewDamage) * (flBonusRatio * flHeavyRatio)) > iArmor)
			flNewDamage = new_damage - (iArmor / flBonusRatio);

		new_damage = flNewDamage;
	}

	damage = new_damage;
}

bool CAutoWall::VectortoVectorVisible(Vector src, Vector point) {

	CGameTrace TraceInit;
	TraceLine(src, point, MASK_SOLID, csgo->local, &TraceInit);

	CGameTrace Trace;
	TraceLine(src, point, MASK_SOLID, TraceInit.m_pEnt, &Trace);

	if (Trace.fraction == 1.0f || TraceInit.fraction == 1.0f)
		return true;

	return false;
};

bool CAutoWall::HandleBulletPenetration(CCSWeaponInfo* info, FireBulletData_t& data, bool extracheck, Vector point)
{
	CGameTrace trace_exit;
	surfacedata_t* enter_surface_data = interfaces.phys_props->GetSurfaceData(data.m_enter_trace.surface.surfaceProps);
	int enter_material = enter_surface_data->game.material;

	float enter_surf_penetration_modifier = enter_surface_data->game.flPenetrationModifier;
	float final_damage_modifier = 0.18f;
	float compined_penetration_modifier = 0.f;

	auto nodraw = (data.m_enter_trace.surface.flags & SURF_NODRAW);
	auto grate = (data.m_enter_trace.contents & CONTENTS_GRATE);

	if (enter_surf_penetration_modifier < 0.1f)
		return false;

	if (!data.m_penetration_count && !nodraw && !grate && enter_material != CHAR_TEX_GRATE && enter_material != CHAR_TEX_GLASS)
		return false;

	if (info->m_flPenetration <= 0.f)
		return false;

	if (!TraceToExit(data.m_enter_trace, trace_exit, data.m_enter_trace.endpos, data.m_direction)) {
		if (!(interfaces.trace->GetPointContents(data.m_enter_trace.endpos, MASK_SHOT_HULL) & MASK_SHOT_HULL))
			return false;
	}

	surfacedata_t* exit_surface_data = interfaces.phys_props->GetSurfaceData(trace_exit.surface.surfaceProps);
	int exit_material = exit_surface_data->game.material;
	float exit_surf_penetration_modifier = exit_surface_data->game.flPenetrationModifier;

	if (enter_material == CHAR_TEX_GRATE || enter_material == CHAR_TEX_GLASS) {
		compined_penetration_modifier = 3.f;
		final_damage_modifier = 0.06f;
	}

	else if (nodraw || grate)
	{
		compined_penetration_modifier = 1.f;
		final_damage_modifier = 0.16f;
	}
	else {
		compined_penetration_modifier = (enter_surf_penetration_modifier + exit_surf_penetration_modifier) * 0.5f;
		final_damage_modifier = 0.16f;
	}

	if (enter_material == exit_material)
	{
		if (exit_material == CHAR_TEX_CARDBOARD || exit_material == CHAR_TEX_WOOD)
			compined_penetration_modifier = 3.f;
		else if (exit_material == CHAR_TEX_PLASTIC)
			compined_penetration_modifier = 2.0f;
	}

	float thickness = (trace_exit.endpos - data.m_enter_trace.endpos).LengthSqr();
	float modifier = fmaxf(1.f / compined_penetration_modifier, 0.f);

	if (extracheck && !VectortoVectorVisible(trace_exit.endpos, point))
		return false;

	float lost_damage = fmaxf(((modifier * thickness) / 24.f) +
		((data.m_current_damage * final_damage_modifier) +
			(fmaxf(3.75f / info->m_flPenetration, 0.f) * 3.f * modifier)), 0.f);

	if (lost_damage > data.m_current_damage)
		return false;

	if (lost_damage > 0.f)
		data.m_current_damage -= lost_damage;

	if (data.m_current_damage < 1.f)
		return false;

	data.m_current_position = trace_exit.endpos;
	data.m_penetration_count--;

	return true;
}

bool CAutoWall::BreakableEntity(IBasePlayer* entity)
{
	if (!entity)
		return false;

	ClientClass* pClass = (ClientClass*)entity->GetClientClass();

	if (!pClass)
	{
		return false;
	}

	return pClass->m_ClassID == 30 || pClass->m_ClassID == 31;

}

bool CAutoWall::TraceToExit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, Vector direction)
{
	Vector start = Vector(0, 0, 0);
	Vector end = Vector(0, 0, 0);
	float maxDistance = 90.f, rayExtension = 4.f, currentDistance = 0;
	int firstContents = 0;

	while (currentDistance <= maxDistance)
	{
		currentDistance += rayExtension;

		start = startPosition + direction * currentDistance;

		if (!firstContents)
		{
			firstContents = interfaces.trace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);
		}

		int pointContents = interfaces.trace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);

		if (!(pointContents & MASK_SHOT_HULL) || (pointContents & CONTENTS_HITBOX && pointContents != firstContents))
		{
			end = start - (direction * rayExtension);

			TraceLine(start, end, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr, &exitTrace);

			if (exitTrace.startsolid && exitTrace.surface.flags & SURF_HITBOX)
			{
				TraceLine(start, startPosition, MASK_SHOT_HULL, exitTrace.m_pEnt, &exitTrace);

				if (exitTrace.DidHit() && !exitTrace.startsolid)
				{
					start = exitTrace.endpos;
					return true;
				}
				continue;
			}

			if (exitTrace.DidHit() && !exitTrace.startsolid)
			{

				if (BreakableEntity(enterTrace.m_pEnt) && BreakableEntity(exitTrace.m_pEnt))
				{
					return true;
				}

				if (enterTrace.surface.flags & SURF_NODRAW || (!(exitTrace.surface.flags & SURF_NODRAW) && exitTrace.plane.normal.Dot(direction) <= 1.0f))
				{
					const float multAmount = exitTrace.fraction * 4.f;
					start -= direction * multAmount;
					return true;
				}

				continue;
			}

			if (!exitTrace.DidHit() || exitTrace.startsolid)
			{
				if (enterTrace.DidHitNonWorldEntity() && BreakableEntity(enterTrace.m_pEnt))
				{
					exitTrace = enterTrace;
					exitTrace.endpos = start + direction;
					return true;
				}

				continue;
			}
		}
	}
	return false;
}

void CAutoWall::TraceLine(Vector& start, Vector& end, unsigned int mask, IBasePlayer* ignore, CGameTrace* trace)
{
	Ray_t ray;
	ray.Init(start, end);

	CTraceFilter filter;
	filter.pSkip = ignore;

	interfaces.trace->TraceRay(ray, mask, &filter, trace);
}

void CAutoWall::ClipTrace(Vector& start, Vector& end, IBasePlayer* e, unsigned int mask, CTraceFilter* filter, CGameTrace* old_trace)
{
	if (!e)
		return;

	Vector mins = e->GetCollideable()->OBBMins(), maxs = e->GetCollideable()->OBBMaxs();

	Vector dir(end - start);
	dir = dir.NormalizeInPlace();

	Vector center = (maxs + mins) / 2, pos(center + e->GetOrigin());

	Vector to = pos - start;
	float range_along = dir.Dot(to);

	float range;

	if (range_along < 0.f)
	{
		range = -to.Length();
	}
	else if (range_along > dir.Length())
	{
		range = -(pos - end).Length();
	}
	else
	{
		auto ray(pos - (dir * range_along + start));
		range = ray.Length();
	}

	if (range <= 60.f)
	{
		CGameTrace trace;

		Ray_t ray;
		ray.Init(start, end);

		interfaces.trace->ClipRayToEntity(ray, mask, e, &trace);

		if (old_trace->fraction > trace.fraction) *old_trace = trace;
	}
}

bool CAutoWall::CanHitFloatingPoint(const Vector& point, const Vector& source) {

	FireBulletData_t data;
	data.m_start = source;
	data.m_filter = new CTraceFilter();
	data.m_filter->pSkip = csgo->local;
	Vector angles = Math::CalculateAngle(data.m_start, point);
	Math::AngleVectors(angles, &data.m_direction);
	data.m_direction = data.m_direction.NormalizeInPlace();
	data.m_penetration_count = 1;

	auto weaponData = csgo->weapon->GetCSWpnData();

	if (!weaponData)
		return false;

	data.m_current_damage = (float)weaponData->m_iDamage;
	Vector end = data.m_start + (data.m_direction * weaponData->m_flRange);
	TraceLine(data.m_start, end, MASK_SHOT_HULL | CONTENTS_HITBOX, csgo->local, &data.m_enter_trace);

	if (VectortoVectorVisible(data.m_start, point) || HandleBulletPenetration(weaponData, data, true, point))
		return true;

	return false;
}