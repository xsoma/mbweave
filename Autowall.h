#pragma once

#define DAMAGE_NO		0
#define DAMAGE_EVENTS_ONLY	1
#define DAMAGE_YES		2
#define DAMAGE_AIM		3

enum {
	CHAR_TEX_ANTLION = 'A',
	CHAR_TEX_BLOODYFLESH = 'B',
	CHAR_TEX_CONCRETE = 'C',
	CHAR_TEX_DIRT = 'D',
	CHAR_TEX_EGGSHELL = 'E',
	CHAR_TEX_FLESH = 'F',
	CHAR_TEX_GRATE = 'G',
	CHAR_TEX_ALIENFLESH = 'H',
	CHAR_TEX_CLIP = 'I',
	CHAR_TEX_PLASTIC = 'L',
	CHAR_TEX_METAL = 'M',
	CHAR_TEX_SAND = 'N',
	CHAR_TEX_FOLIAGE = 'O',
	CHAR_TEX_COMPUTER = 'P',
	CHAR_TEX_SLOSH = 'S',
	CHAR_TEX_TILE = 'T',
	CHAR_TEX_CARDBOARD = 'U',
	CHAR_TEX_VENT = 'V',
	CHAR_TEX_WOOD = 'W',
	CHAR_TEX_GLASS = 'Y',
	CHAR_TEX_WARPSHIELD = 'Z',
};
struct ReturnInfo_t
{
	int m_damage;
	int m_hitgroup;
	int m_penetration_count;
	bool m_did_penetrate_wall;
	float m_thickness;
	Vector m_end;
	IBasePlayer* m_hit_entity;

	ReturnInfo_t(int damage, int hitgroup, int penetration_count, bool did_penetrate_wall, float thickness, IBasePlayer* hit_entity)
	{
		m_damage = damage;
		m_hitgroup = hitgroup;
		m_penetration_count = penetration_count;
		m_did_penetrate_wall = did_penetrate_wall;
		m_thickness = thickness;
		m_hit_entity = hit_entity;
	}
};

class CAutoWall
{
private:
	struct FireBulletData_t
	{
		Vector m_start;
		Vector m_end;
		Vector m_current_position;
		Vector m_direction;

		CTraceFilter* m_filter;
		CGameTrace m_enter_trace;

		float m_current_damage;
		int m_penetration_count;
	};

	void ScaleDamage(IBasePlayer* e, CCSWeaponInfo* weapon_info, int hitgroup, float& current_damage);
	bool VectortoVectorVisible(Vector src, Vector point);
	bool HandleBulletPenetration(CCSWeaponInfo* info, FireBulletData_t& data, bool extracheck = false, Vector point = Vector(0, 0, 0));
	bool BreakableEntity(IBasePlayer* entity);
	bool TraceToExit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, Vector direction);
	void TraceLine(Vector& start, Vector& end, unsigned int mask, IBasePlayer* ignore, CGameTrace* trace);
	void ClipTrace(Vector& start, Vector& end, IBasePlayer* e, unsigned int mask, CTraceFilter* filter, CGameTrace* old_trace);

	float HitgroupDamage(int iHitGroup);

	bool IsArmored(IBasePlayer* player, int hitgroup);

public:
	std::vector<float> scanned_damage;
	std::vector<Vector> scanned_points;
	void reset() {
		scanned_damage.clear();
		scanned_points.clear();
	}
	bool CanHitFloatingPoint(const Vector& point, const Vector& source);
	ReturnInfo_t Think(Vector pos, IBasePlayer* target, int specific_hitgroup = -1, Vector eye_pos = csgo->eyepos, IBasePlayer* start_ent = csgo->local);
};