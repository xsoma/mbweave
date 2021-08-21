#pragma once
#include "Hooks.h"
#include <unordered_map>
#include <optional>
#include <deque>

#define LEFT_SIDE 0
#define RIGHT_SIDE 1

struct animation
{
	animation() = default;
	explicit animation(IBasePlayer * player);
	explicit animation(IBasePlayer * player, Vector last_reliable_angle);
	void restore(IBasePlayer * player) const;
	void apply(IBasePlayer * player) const;
	void build_unresolved_bones(IBasePlayer * player);
	void build_inversed_bones(IBasePlayer * player);
	void build_server_bones(IBasePlayer * player);
	bool is_valid(float range, float max_unlag);
	bool is_valid_extended();

	IBasePlayer* player{};
	int32_t index{};

	bool valid{}, has_anim_state{};
	alignas(16) matrix bones[128];
	alignas(16) matrix unresolved_bones[128];
	alignas(16) matrix inversed_bones[128];

	bool dormant{};
	uint8_t scan_mode = 128;
	Vector velocity;
	Vector origin;
	matrix* bone_cache;
	Vector abs_origin;
	Vector obb_mins;
	Vector obb_maxs;
	CAnimationLayer layers[13];

	std::array<float, 24> poses;
	CCSGOPlayerAnimState* anim_state;
	float anim_time{};
	float sim_time{};
	float interp_time{};
	float duck{};
	float lby{};
	float last_shot_time{};
	Vector last_reliable_angle{};
	Vector eye_angles;
	Vector abs_ang;
	int flags{};
	int eflags{};
	int effects{};
	float m_flFeetCycle{};
	float m_flFeetYawRate{};
	int lag{};
	bool didshot;
	bool safepoints;
	int came_from_dormant = -1;
	int priority = -1;
	std::string resolver_mode;

	float land_time = 0.0f;

	bool is_landed = false;
	bool land_in_cycle = false;
};

class CAnimationFix
{
public:
	struct animation_info {
		animation_info(IBasePlayer* player, std::deque<animation> animations)
			: player(player), frames(std::move(animations)), last_spawn_time(0) { }

		void UpdateAnims(animation* record, animation* from);

		IBasePlayer* player{};
		std::deque<animation> frames;

		// last time this player spawned
		float last_spawn_time;
		float goal_feet_yaw;
		Vector last_reliable_angle;
		float air_time = 0.f;
		float last_landtime;
	};

	unordered_map<CBaseHandle, animation_info*> animation_infos;

	CCSGOPlayerAnimState* US_Animstate[64];
	CCSGOPlayerAnimState* IS_Animstate[64];

	void UpdateRealState();

	void FixPvs();

	void UpdatePlayers();
	void UpdateFakeState();

	CCSGOPlayerAnimState* FakeAnimstate = nullptr;
	CCSGOPlayerAnimState* RealAnimstate = nullptr;

	animation_info* get_animation_info(IBasePlayer* player);
	animation* get_latest_animation(IBasePlayer* player);
	std::vector<animation*> get_valid_animations(IBasePlayer* player);
	animation* get_oldest_animation(IBasePlayer* player);
	animation* get_latest_firing_animation(IBasePlayer* player);
};