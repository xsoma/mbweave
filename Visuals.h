#pragma once
#include "Hooks.h"
#include "RageBackTracking.h"
#include "Hitmarker.h"
#include <list>
#include <map>

enum zeus_stages {
	none = -1,
	good,
	warning,
	fatal
};

struct BOX
{
	int h, w, x, y;
};
struct BaseData {
	BaseData(IBasePlayer* entity) noexcept;

	constexpr auto operator<(const BaseData& other) const
	{
		return distanceToLocal > other.distanceToLocal;
	}

	float distanceToLocal;
	Vector obbMins, obbMaxs;
	Vector origin;
	matrix coordinateFrame;
	struct {
		int x, y, w, h;
	} box;
	float time_to_die = 0.f;
	bool m_bDidSmokeEffect;
	int m_nSmokeEffectTickBegin;
};

struct ColorToggleThickness {
	ColorToggleThickness() = default;
	ColorToggleThickness(float thickness) : thickness{ thickness } { }
	float thickness = 1.0f;
};
struct Trail : ColorToggleThickness {
	enum Type {
		Line = 0,
		Circles,
		FilledCircles
	};
	color_t color = color_t(255, 255, 255, 255);
	bool rainbow;
	int type = Line;
	float time = 2.0f;
};
struct WeaponData_t : BaseData {
	WeaponData_t(IBasePlayer *entity) noexcept;

	int clip;
	int reserveAmmo;
	std::string group = str("All");
	std::string name = str("All");
	std::string displayName;
};
struct ProjectileInfo_t : BaseData {
	ProjectileInfo_t(IBasePlayer* projectile) noexcept;

	void update(IBasePlayer* projectile) noexcept;

	constexpr auto operator==(int otherHandle) const noexcept
	{
		return handle == otherHandle;
	}

	bool exploded = false;
	bool thrownByLocalPlayer = false;
	bool thrownByEnemy = false;
	int handle;
	std::string name = str("");
	std::vector<std::pair<float, Vector>> trajectory;
	bool trail_added = false;
	IBasePlayer* entity;
};
struct InfernoInfo_t {
	std::vector<Vector> points;
	Vector entity_origin;
	Vector origin;
	float time_to_die;
	float range;
};
struct Flags_t
{
	std::string name = "";
	bool toggled = false;
	bool icon = false;
	float modifier = 0.f;
	color_t clr = color_t();

	Flags_t(std::string cur_name, bool toggle, color_t color, bool is_icon = false) {
		name = cur_name;
		toggled = toggle;
		clr = color;
		icon = is_icon;
	}
};

struct Health_t
{
	int health = -1;
	float difference = 0;
	float multiplier = 0;

	void Erase()
	{
		health = -1;
		difference = 0.f;
		multiplier = 0.f;
	}
};
struct INFO_t
{
	IBasePlayer* player;
	std::string weapon_name;
	IBaseCombatWeapon* weapon;
	int hp;
	int ammo, max_ammo;

	int player_distance;

	bool offscreen;
	int zeuser_stages;
	bool is_valid = false;
	bool is_gun;

	bool scope,
		flash, have_armor, have_helmet, dormant,
		have_kit, fake_duck, misc_weapon;

	std::string name, last_place;
	Vector origin;
	float angle1, angle2;
	float duck, last_seen_time;
	Vector body_pos;
	float alpha;
	struct {
		int x, y, w, h;
	}box;
	struct
	{
		std::vector<int> hitboxes;
		std::vector<Vector> points;
		matrix bones[128];
	}AnimInfo;

	studiohdr_t* hdr;

	Vector bone_pos_child[128];
	Vector bone_pos_parent[128];

	std::map<int, float> modifier = {};
	std::map<int, float> esp_offsets = {};

	Health_t health_info;

	void Reset() {
		player = nullptr;
		weapon = nullptr;
		hdr = nullptr;

		memset(AnimInfo.bones, 0, sizeof(AnimInfo.bones));

		if (!AnimInfo.hitboxes.empty())
			AnimInfo.hitboxes.clear();

		if (!AnimInfo.points.empty())
			AnimInfo.points.clear();

		if (name.length())
			name.clear();

		if (last_place.length())
			last_place.clear();

		if (weapon_name.length())
			weapon_name.clear();

		esp_offsets.clear();
		modifier.clear();
		health_info.Erase();

		hp = 0;
		ammo = 0;
		max_ammo = 0;
		duck = 0.f;
		last_seen_time = 0.f;
		player_distance = 0;
		alpha = 0.f;
		box.x = 0;
		box.y = 0;
		box.w = 0;
		box.h = 0;

		zeuser_stages = -1;

		is_valid = false;
		is_gun = false;
		scope = false;
		flash = false;
		have_armor = false;
		have_helmet = false;
		dormant = false;
		have_kit = false;
		fake_duck = false;
		misc_weapon = false;

		origin = Vector(0, 0, 0);
		body_pos = Vector(0, 0, 0);

		memset(csgo->EntityAlpha, 0, sizeof(csgo->EntityAlpha));
		memset(bone_pos_child, 0, sizeof(bone_pos_child));
		memset(bone_pos_parent, 0, sizeof(bone_pos_parent));

		offscreen = false;
	}
};

struct BombInfo_t {
	Vector origin;
	float time;
	float blow;
	float blow_time;
	float bomb_ticking;
	float defuse_time;
	float defuse_cooldown;
	bool is_defusing;
	bool bomb_defused;
};

class CVisuals
{
public:
	std::list<ProjectileInfo_t> ProjectileInfo;
	std::vector<WeaponData_t> WeaponData;
	std::vector<InfernoInfo_t> InfernoInfo;
	std::vector<BombInfo_t> BombInfo;
	std::vector<Flags_t> FlagsInfo[64] = {};

	Vector PredictedOrigin;

	int health[65];

	INFO_t player_info[64] = {};
	void DrawWatermark();
	void DrawLocalVisuals();
	void Draw();
	void StoreOtherInfo();
	void ResetInfo();
	void RecordInfo();
	void DrawAngleLine(Vector origin, float angle, color_t color);
	Vector strored_origin[64];
};

namespace Z {
	enum class WeaponType {
		Knife = 0,
		Pistol,
		SubMachinegun,
		Rifle,
		Shotgun,
		SniperRifle,
		Machinegun,
		C4,
		Placeholder,
		Grenade,
		Unknown,
		StackableItem,
		Fists,
		BreachCharge,
		BumpMine,
		Tablet,
		Melee
	};

	enum class WeaponId : short {
		Deagle = 1,
		Elite,
		Fiveseven,
		Glock,
		Ak47 = 7,
		Aug,
		Awp,
		Famas,
		G3SG1,
		GalilAr = 13,
		M249,
		M4A1 = 16,
		Mac10,
		P90 = 19,
		ZoneRepulsor,
		Mp5sd = 23,
		Ump45,
		Xm1014,
		Bizon,
		Mag7,
		Negev,
		Sawedoff,
		Tec9,
		Taser,
		Hkp2000,
		Mp7,
		Mp9,
		Nova,
		P250,
		Shield,
		Scar20,
		Sg553,
		Ssg08,
		GoldenKnife,
		Knife,
		Flashbang = 43,
		HeGrenade,
		SmokeGrenade,
		Molotov,
		Decoy,
		IncGrenade,
		C4,
		Healthshot = 57,
		KnifeT = 59,
		M4a1_s,
		Usp_s,
		Cz75a = 63,
		Revolver,
		TaGrenade = 68,
		Axe = 75,
		Hammer,
		Spanner = 78,
		GhostKnife = 80,
		Firebomb,
		Diversion,
		FragGrenade,
		Snowball,
		BumpMine,
		Bayonet = 500,
		ClassicKnife = 503,
		Flip = 505,
		Gut,
		Karambit,
		M9Bayonet,
		Huntsman,
		Falchion = 512,
		Bowie = 514,
		Butterfly,
		Daggers,
		Paracord,
		SurvivalKnife,
		Ursus = 519,
		Navaja,
		NomadKnife,
		Stiletto = 522,
		Talon,
		SkeletonKnife = 525,
		GloveStuddedBloodhound = 5027,
		GloveT,
		GloveCT,
		GloveSporty,
		GloveSlick,
		GloveLeatherWrap,
		GloveMotorcycle,
		GloveSpecialist,
		GloveHydra
	};

	enum class ClassId {
		BaseCSGrenadeProjectile = 9,
		BreachChargeProjectile = 29,
		BumpMineProjectile = 33,
		C4,
		Chicken = 36,
		CSPlayer = 40,
		CSPlayerResource,
		CSRagdoll = 42,
		Deagle = 46,
		DecoyProjectile = 48,
		Drone,
		Dronegun,
		DynamicProp = 52,
		EconEntity = 53,
		EconWearable,
		Hostage = 97,
		Healthshot = 104,
		Cash,
		Knife = 107,
		KnifeGG,
		MolotovProjectile = 114,
		AmmoBox = 125,
		LootCrate,
		RadarJammer,
		WeaponUpgrade,
		PlantedC4,
		PropDoorRotating = 143,
		SensorGrenadeProjectile = 153,
		SmokeGrenadeProjectile = 157,
		SnowballPile = 160,
		SnowballProjectile,
		Tablet = 172,
		Aug = 232,
		Awp,
		Elite = 239,
		FiveSeven = 241,
		G3sg1,
		Glock = 245,
		P2000,
		P250 = 258,
		Scar20 = 261,
		Sg553 = 265,
		Ssg08 = 267,
		Tec9 = 269
	};

}