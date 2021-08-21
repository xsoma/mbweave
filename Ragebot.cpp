#include "Hooks.h"
#include "features.h"
#include <thread>
#include <mutex>

std::vector<ShotSnapshot> shot_snapshots;

CWeaponConfig CRagebot::CurrentSettings() {
	if (csgo->weapon->IsAuto() && vars.ragebot.weapon[weap_type::scar].enable)
		return vars.ragebot.weapon[weap_type::scar];
	else if (csgo->weapon->GetItemDefinitionIndex() == WEAPON_SSG08 && vars.ragebot.weapon[weap_type::scout].enable)
		return vars.ragebot.weapon[weap_type::scout];
	else if (csgo->weapon->GetItemDefinitionIndex() == WEAPON_AWP && vars.ragebot.weapon[weap_type::_awp].enable)
		return vars.ragebot.weapon[weap_type::_awp];
	else if (csgo->weapon->isRifle() && vars.ragebot.weapon[weap_type::rifles].enable)
		return vars.ragebot.weapon[weap_type::rifles];
	else if (csgo->weapon->IsHeavyPistol() && vars.ragebot.weapon[weap_type::heavy_pistols].enable)
		return vars.ragebot.weapon[weap_type::heavy_pistols];
	else if (csgo->weapon->isPistol() && vars.ragebot.weapon[weap_type::pistols].enable)
		return vars.ragebot.weapon[weap_type::pistols];
	else
		return vars.ragebot.weapon[weap_type::def];
}

std::vector<int> CRagebot::GetHitboxesToScan(IBasePlayer* pEntity)
{
	std::vector< int > hitboxes;

	if (!csgo->weapon)
		return hitboxes;

	if (g_Binds[bind_baim].active) {
		hitboxes.emplace_back(CSGOHitboxID::Chest);
		hitboxes.emplace_back(CSGOHitboxID::Stomach);
		hitboxes.emplace_back(CSGOHitboxID::Pelvis);
		hitboxes.emplace_back(CSGOHitboxID::UpperChest);
		return hitboxes;
	}

	if (csgo->weapon->IsZeus()) {
		hitboxes.emplace_back(CSGOHitboxID::Chest);
		hitboxes.emplace_back(CSGOHitboxID::Stomach);
		hitboxes.emplace_back(CSGOHitboxID::Pelvis);
		return hitboxes;
	}

	if (current_settings.hitscan == 666/*devil*/)
	{
		hitboxes.emplace_back(CSGOHitboxID::Head);
		hitboxes.emplace_back(CSGOHitboxID::UpperChest);
		hitboxes.emplace_back(CSGOHitboxID::Chest);
		hitboxes.emplace_back(CSGOHitboxID::LowerChest);
		hitboxes.emplace_back(CSGOHitboxID::Pelvis);
		hitboxes.emplace_back(CSGOHitboxID::Stomach);
		hitboxes.emplace_back(CSGOHitboxID::LeftShin);
		hitboxes.emplace_back(CSGOHitboxID::RightShin);
		hitboxes.emplace_back(CSGOHitboxID::LeftThigh);
		hitboxes.emplace_back(CSGOHitboxID::RightThigh);
		hitboxes.emplace_back(CSGOHitboxID::LeftFoot);
		hitboxes.emplace_back(CSGOHitboxID::RightFoot);
		hitboxes.emplace_back(CSGOHitboxID::LeftHand);
		hitboxes.emplace_back(CSGOHitboxID::RightHand);
		hitboxes.emplace_back(CSGOHitboxID::LeftLowerArm);
		hitboxes.emplace_back(CSGOHitboxID::RightLowerArm);
		hitboxes.emplace_back(CSGOHitboxID::LeftUpperArm);
		hitboxes.emplace_back(CSGOHitboxID::RightUpperArm);
	}

	if (current_settings.hitscan & 1)
		hitboxes.emplace_back(CSGOHitboxID::Head);

	if (current_settings.hitscan & 2)
		hitboxes.emplace_back(CSGOHitboxID::UpperChest);

	if (current_settings.hitscan & 4)
		hitboxes.emplace_back(CSGOHitboxID::Chest);

	if (current_settings.hitscan & 8)
		hitboxes.emplace_back(CSGOHitboxID::LowerChest);

	if (current_settings.hitscan & 16)
		hitboxes.emplace_back(CSGOHitboxID::Pelvis);

	if (current_settings.hitscan & 32)
		hitboxes.emplace_back(CSGOHitboxID::Stomach);

	if (csgo->weapon->isSniper() && !csgo->local->IsScoped())
		return hitboxes;

	if (current_settings.hitscan & 64) {
		hitboxes.emplace_back(CSGOHitboxID::LeftShin);
		hitboxes.emplace_back(CSGOHitboxID::RightShin);
		hitboxes.emplace_back(CSGOHitboxID::LeftThigh);
		hitboxes.emplace_back(CSGOHitboxID::RightThigh);
	}

	if (current_settings.hitscan & 128) {
		hitboxes.emplace_back(CSGOHitboxID::LeftFoot);
		hitboxes.emplace_back(CSGOHitboxID::RightFoot);
	}

	if (current_settings.hitscan & 256) {
		hitboxes.emplace_back(CSGOHitboxID::LeftHand);
		hitboxes.emplace_back(CSGOHitboxID::RightHand);
		hitboxes.emplace_back(CSGOHitboxID::LeftLowerArm);
		hitboxes.emplace_back(CSGOHitboxID::RightLowerArm);
		hitboxes.emplace_back(CSGOHitboxID::LeftUpperArm);
		hitboxes.emplace_back(CSGOHitboxID::RightUpperArm);
	}
	return hitboxes;
}

float CRagebot::GetBodyScale(IBasePlayer* player)
{
	if (!(player->GetFlags() & FL_ONGROUND))
		return 0.f;

	if (current_settings.static_scale)
		return std::clamp(current_settings.scale_body / 100.f, 0.f, 0.75f);

	auto factor = [](float x, float min, float max) {
		return 1.f - 1.f / (1.f + pow(2.f, (-([](float x, float min, float max) {
			return ((x - min) * 2.f) / (max - min) - 1.f;
			}(x, min, max)) / 0.115f)));
	}(player->GetOrigin().DistTo(csgo->local->GetEyePosition()), 0.f, csgo->weapon_range / 4.f);

	if (csgo->weapon->isSniper() && !csgo->local->IsScoped())
		factor = 0.f;

	if (csgo->local->GetDuckAmount() >= 0.9f && !g_Binds[bind_fake_duck].active)
		return 0.65f;

	return std::clamp(factor, 0.f, 0.75f);
}

float CRagebot::GetHeadScale(IBasePlayer* player)
{
	if (current_settings.static_scale)
		return std::clamp(current_settings.scale_head / 100.f, 0.f, 0.80f);

	if (!vars.misc.antiuntrusted)
		return 0.90f;

	if (g_Binds[bind_fake_duck].active)
		return 0.70f;

	if (player->GetFlags() & FL_ONGROUND)
		return GetBodyScale(player);
	else
		return 0.75f;
}

std::vector<Vector> CRagebot::GetAdvancedHeadPoints(IBasePlayer* pBaseEntity, matrix bones[128])
{
	std::vector<Vector> vPoints;

	Vector src, dst, sc1, sc2, fw1;

	src = pBaseEntity->GetBonePos(bones, 8);
	Math::AngleVectors(Vector(0, g_Resolver->GetBackwardYaw(pBaseEntity) - 90.f, 0), &fw1);

	Vector left_side = src + (fw1 * 40);
	Vector right_side = src - (fw1 * 40);

	vPoints.push_back(left_side);
	vPoints.push_back(right_side);

	return vPoints;
}

std::vector<std::pair<Vector, bool>> CRagebot::GetMultipoints(IBasePlayer* pBaseEntity, int iHitbox, matrix bones[128])
{
	std::vector<std::pair<Vector, bool>> points;

	const model_t* model = pBaseEntity->GetModel();
	if (!model)
		return points;

	studiohdr_t* hdr = interfaces.models.model_info->GetStudioModel(model);
	if (!hdr)
		return points;

	mstudiohitboxset_t* set = hdr->pHitboxSet(pBaseEntity->GetHitboxSet());
	if (!set)
		return points;

	mstudiobbox_t* bbox = set->pHitbox(iHitbox);
	if (!bbox)
		return points;

	// these indexes represent boxes.
	if (bbox->radius <= 0.f) {
		// references: 
		//      https://developer.valvesoftware.com/wiki/Rotation_Tutorial
		//      CBaseAnimating::GetHitboxBonePosition
		//      CBaseAnimating::DrawServerHitboxes

		// convert rotation angle to a matrix.
		matrix rot_matrix;
		Math::AngleMatrix(bbox->rotation, rot_matrix);

		// apply the rotation to the entity input space (local).
		matrix mat;
		Math::ConcatTransforms(bones[bbox->bone], rot_matrix, mat);

		// extract origin from matrix.
		Vector origin = mat.GetOrigin();

		// compute raw center point.
		Vector center = (bbox->bbmin + bbox->bbmax) / 2.f;

		// the feet hiboxes have a side, heel and the toe.
		if (iHitbox == CSGOHitboxID::RightFoot || iHitbox == CSGOHitboxID::LeftFoot) {
			float d1 = (bbox->bbmin.z - center.z) * 0.875f;

			// invert.
			if (iHitbox == CSGOHitboxID::LeftFoot)
				d1 *= -1.f;

			// side is more optimal then center.
			points.emplace_back(Vector{ center.x, center.y, center.z + d1 }, false);

			float d2 = (bbox->bbmin.x - center.x) * 0.5f;
			float d3 = (bbox->bbmax.x - center.x) * 0.5f;

			// heel.
			points.emplace_back(Vector{ center.x + d2, center.y, center.z }, false);

			// toe.
			points.emplace_back(Vector{ center.x + d3, center.y, center.z }, false);

		}
		else
			points.emplace_back(center, true);
		// nothing to do here we are done.
		if (points.empty())
			return points;

		// rotate our bbox points by their correct angle
		// and convert our points to world space.
		for (auto& p : points) {
			// VectorRotate.
			// rotate point by angle stored in matrix.
			p.first = { p.first.Dot(mat[0]), p.first.Dot(mat[1]), p.first.Dot(mat[2]) };

			// transform point to world space.
			p.first += origin;
		}
	}

	// these hitboxes are capsules.
	else {
		// compute raw center point.
		Vector max = bbox->bbmax;
		Vector min = bbox->bbmin;
		Vector center = (bbox->bbmin + bbox->bbmax) / 2.f;

		// head has 5 points.
		if (iHitbox == CSGOHitboxID::Head) {
			// add center.
			float r = bbox->radius * GetHeadScale(pBaseEntity);
			points.emplace_back(center, true);

			if (true) {
				// rotation matrix 45 degrees.
				// https://math.stackexchange.com/questions/383321/rotating-x-y-points-45-degrees
				// std::cos( deg_to_rad( 45.f ) )
				constexpr float rotation = 0.70710678f;

				// top/back 45 deg.
				// this is the best spot to shoot at.
				points.emplace_back(Vector{ max.x + (rotation * r), max.y + (-rotation * r), max.z }, false);

				Vector right{ max.x, max.y, max.z + r };

				// right.
				points.emplace_back(right, false);

				Vector left{ max.x, max.y, max.z - r };

				// left.
				points.emplace_back(left, false);

				// back.
				points.emplace_back(Vector{ max.x, max.y - r, max.z }, false);

				// get animstate ptr.
				CCSGOPlayerAnimState* state = pBaseEntity->GetPlayerAnimState();

				// add this point only under really specific circumstances.
				// if we are standing still and have the lowest possible pitch pose.
				if (state && pBaseEntity->GetVelocity().Length() <= 0.1f && pBaseEntity->GetEyeAngles().x <= 75.f) {

					// bottom point.
					points.emplace_back(Vector{ max.x - r, max.y, max.z }, false);
				}
			}
		}

		// body has 5 points.
		else {
			float r = bbox->radius * GetBodyScale(pBaseEntity);

			if (iHitbox == CSGOHitboxID::Stomach) {
				// center.
				points.emplace_back(center, true);
				points.emplace_back(Vector(center.x, center.y, min.z + r), false);
				points.emplace_back(Vector(center.x, center.y, max.z - r), false);
				// back.
				points.emplace_back(Vector{ center.x, max.y - r, center.z }, true);
			}

			else if (iHitbox == CSGOHitboxID::Pelvis || iHitbox == CSGOHitboxID::UpperChest) {
				//points.emplace_back(center);
				// left & right points
				points.emplace_back(Vector(center.x, center.y, max.z + r), false);
				points.emplace_back(Vector(center.x, center.y, min.z - r), false);
			}

			// other stomach/chest hitboxes have 2 points.
			else if (iHitbox == CSGOHitboxID::LowerChest || iHitbox == CSGOHitboxID::Chest) {
				// left & right points
				points.emplace_back(Vector(center.x, center.y, max.z + r), false);
				points.emplace_back(Vector(center.x, center.y, min.z - r), false);
				// add extra point on back.
				points.emplace_back(Vector{ center.x, max.y - r, center.z }, false);
			}

			else if (iHitbox == CSGOHitboxID::RightShin || iHitbox == CSGOHitboxID::LeftShin) {
				// add center.
				points.emplace_back(center, true);

				// half bottom.
				points.emplace_back(Vector{ max.x - (bbox->radius / 2.f), max.y, max.z }, false);
			}

			else if (iHitbox == CSGOHitboxID::RightThigh || iHitbox == CSGOHitboxID::LeftThigh) {
				// add center.
				points.emplace_back(center, true);
			}

			// arms get only one point.
			else if (iHitbox == CSGOHitboxID::RightUpperArm || iHitbox == CSGOHitboxID::LeftUpperArm) {
				// elbow.
				points.emplace_back(Vector{ max.x + bbox->radius, center.y, center.z }, false);
			}
			else
				points.emplace_back(center, true);
		}
		// nothing left to do here.
		if (points.empty())
			return points;

		// transform capsule points.
		for (auto& p : points)
			Math::VectorTransform(p.first, bones[bbox->bone], p.first);
	}

	return points;
}
Vector CRagebot::GetPoint(IBasePlayer* pBaseEntity, int iHitbox, matrix BoneMatrix[128])
{
	std::vector<Vector> vPoints;

	if (!pBaseEntity)
		return Vector(0, 0, 0);

	studiohdr_t* pStudioModel = interfaces.models.model_info->GetStudioModel(pBaseEntity->GetModel());
	mstudiohitboxset_t* set = pStudioModel->pHitboxSet(0);

	if (!set)
		return Vector(0, 0, 0);

	mstudiobbox_t* untransformedBox = set->pHitbox(iHitbox);
	if (!untransformedBox)
		return Vector(0, 0, 0);

	Vector vecMin = { 0, 0, 0 };
	Math::VectorTransform_Wrapper(untransformedBox->bbmin, BoneMatrix[untransformedBox->bone], vecMin);

	Vector vecMax = { 0, 0, 0 };
	Math::VectorTransform_Wrapper(untransformedBox->bbmax, BoneMatrix[untransformedBox->bone], vecMax);

	float mod = untransformedBox->radius != -1.f ? untransformedBox->radius : 0.f;
	Vector max;
	Vector min;

	Math::VectorTransform(untransformedBox->bbmax + mod, BoneMatrix[untransformedBox->bone], max);
	Math::VectorTransform(untransformedBox->bbmin - mod, BoneMatrix[untransformedBox->bone], min);

	return (min + max) * 0.5f;
}

uint8_t CRagebot::ScanMode(IBasePlayer* pEntity)
{
	int scan_mode = [&]() -> int {
		auto weapon = csgo->local->GetWeapon();
		if (!weapon || !pEntity->isAlive() || !csgo->local->isAlive())
			return ScanType::SCAN_DEFAULT;

		if (weapon->IsZeus())
			return ScanType::SCAN_FORCE_SAFEPOINT;

		if (g_Binds[bind_baim].active)
			return ScanType::SCAN_SAFE_BAIM;

		if (g_Binds[bind_force_safepoint].active)
			return ScanType::SCAN_FORCE_SAFEPOINT;

		const int& missed_shots = csgo->actual_misses[pEntity->GetIndex()] + std::clamp(csgo->imaginary_misses[pEntity->GetIndex()] - 1, 0, INT_MAX);
		if (current_settings.max_misses > 0 && missed_shots >= current_settings.max_misses)
			return missed_shots == current_settings.max_misses ? ScanType::SCAN_FORCE_SAFEPOINT : ScanType::SCAN_SAFE_BAIM;

		if (current_settings.prefer_safepoint)
			return ScanType::SCAN_PREFER_SAFEPOINT;

		return ScanType::SCAN_DEFAULT;
	}();

	/*
	
	str("Head"), 1
	str("Upper chest"), 2
	str("Chest"), 4
	str("Lower chest"),  8
	str("Pelvis"), 16
	str("Stomach"), 32
	str("Legs"), 64
	str("Feet"), 128
	str("Arms"), 256
	
	*/

	bool body_hitboxes_selected = current_settings.hitscan & 2 || current_settings.hitscan & 4
		|| current_settings.hitscan & 8 || current_settings.hitscan & 16 || current_settings.hitscan & 32;

	if (!body_hitboxes_selected)
		return clamp(scan_mode, (int)ScanType::SCAN_DEFAULT, (int)ScanType::SCAN_PREFER_SAFEPOINT);

	return scan_mode;
}

void CRagebot::BackupPlayer(animation* anims) {
	auto i = anims->player->GetIndex();
	backup_anims[i].origin = anims->player->GetOrigin();
	backup_anims[i].abs_origin = anims->player->GetAbsOrigin();
	backup_anims[i].obb_mins = anims->player->GetMins();
	backup_anims[i].obb_maxs = anims->player->GetMaxs();
	backup_anims[i].bone_cache = anims->player->GetBoneCache().Base();
}

void CRagebot::SetAnims(animation* anims, matrix* m) {
	anims->player->GetOrigin() = anims->origin;
	anims->player->SetAbsOrigin(anims->abs_origin);
	anims->player->GetMins() = anims->obb_mins;
	anims->player->GetMaxs() = anims->obb_maxs;
	anims->player->SetBoneCache(m);
}

void CRagebot::RestorePlayer(animation* anims) {
	auto i = anims->player->GetIndex();
	anims->player->GetOrigin() = backup_anims[i].origin;
	anims->player->SetAbsOrigin(backup_anims[i].abs_origin);
	anims->player->GetMins() = backup_anims[i].obb_mins;
	anims->player->GetMaxs() = backup_anims[i].obb_maxs;
	anims->player->SetBoneCache(backup_anims[i].bone_cache);
}

int HitboxToHitgroup(CSGOHitboxID hitbox) {
	switch (hitbox)
	{
	case CSGOHitboxID::Head: return HITGROUP_HEAD;
	case CSGOHitboxID::Neck: return HITGROUP_NECK;
	case CSGOHitboxID::Pelvis:
	case CSGOHitboxID::Stomach:
		return HITGROUP_STOMACH;
	case CSGOHitboxID::LowerChest:
	case CSGOHitboxID::Chest:
	case CSGOHitboxID::UpperChest:
		return HITGROUP_CHEST;
	case CSGOHitboxID::RightShin:
	case CSGOHitboxID::RightThigh:
	case CSGOHitboxID::RightFoot:
		return HITGROUP_RIGHTLEG;
	case CSGOHitboxID::LeftThigh:
	case CSGOHitboxID::LeftShin:
	case CSGOHitboxID::LeftFoot:
		return HITGROUP_LEFTLEG;
	case CSGOHitboxID::RightUpperArm:
	case CSGOHitboxID::RightLowerArm:
	case CSGOHitboxID::RightHand:
		return HITGROUP_RIGHTARM;
	case CSGOHitboxID::LeftHand:
	case CSGOHitboxID::LeftUpperArm:
	case CSGOHitboxID::LeftLowerArm:
		return HITGROUP_LEFTARM;

	default: return HITGROUP_GENERIC;
	}
}


bool CanHitHitbox(const Vector start, const Vector end, animation* _animation, int box, matrix* bones)
{
	studiohdr_t* pStudioModel = interfaces.models.model_info->GetStudioModel(_animation->player->GetModel());
	mstudiohitboxset_t* set = pStudioModel->pHitboxSet(0);

	if (!set)
		return false;

	mstudiobbox_t* studio_box = set->pHitbox(box);
	if (!studio_box)
		return false;

	Vector min, max;

	const auto is_capsule = studio_box->radius != -1.f;

	if (is_capsule)
	{
		Math::VectorTransform(studio_box->bbmin, bones[studio_box->bone], min);
		Math::VectorTransform(studio_box->bbmax, bones[studio_box->bone], max);

		const auto dist = Math::segment_to_segment(start, end, min, max);

		if (dist < studio_box->radius) {
			if (box == CSGOHitboxID::Head) {
				g_Ragebot->BackupPlayer(_animation);
				g_Ragebot->SetAnims(_animation, bones);

				trace_t tr;
				Ray_t ray;
				ray.Init(start, end);
				interfaces.trace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, _animation->player, &tr);

				g_Ragebot->RestorePlayer(_animation);

				return tr.m_pEnt == _animation->player && tr.hitgroup == HITGROUP_HEAD;
			}
			return true;
		}
	}
	else
	{
		g_Ragebot->BackupPlayer(_animation);
		g_Ragebot->SetAnims(_animation, bones);

		trace_t tr;
		Ray_t ray;
		ray.Init(start, end);
		interfaces.trace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, _animation->player, &tr);

		g_Ragebot->RestorePlayer(_animation);
		if (auto ent = tr.m_pEnt; ent)
		{
			if (ent == _animation->player) {
				if (HitboxToHitgroup((CSGOHitboxID)box) == tr.hitgroup)
					return true;
			}
		}
	}
	return false;
}

static const auto& is_baim_hitbox = [](int id) {
	static auto baim_hitbox = {
		CSGOHitboxID::Pelvis,
		CSGOHitboxID::Stomach,
		CSGOHitboxID::LowerChest,
		CSGOHitboxID::Chest,
		CSGOHitboxID::UpperChest,
	};
	bool found = false;
	for (const auto& hitbox : baim_hitbox) {
		if ((int)hitbox == id) {
			found = true;
			break;
		}
	}
	return found;
};

CPoint CRagebot::GetBestPoint(std::vector<CPoint> points, int health, uint8_t scan_mode) {

	if (points.empty())
		return CPoint{};

	static auto check_safety = [&](int hitbox, bool force = false) {
		if (g_Binds[bind_force_safepoint].active)
			return true;

		bool head_safe = false, body_safe = false, limbs_safe = false;

		if (force) {
			head_safe = current_settings.force_safepoint & 1;
			body_safe = current_settings.force_safepoint & 2;
			limbs_safe = current_settings.force_safepoint & 4;
		}
		else {
			head_safe = current_settings.prefer_safepoint & 1;
			body_safe = current_settings.prefer_safepoint & 2;
			limbs_safe = current_settings.prefer_safepoint & 4;
		}

		switch (hitbox)
		{
		case Head:
			return head_safe;
		case Pelvis:
		case Stomach:
		case LowerChest:
		case Chest:
		case UpperChest:
			return body_safe;
		case RightThigh:
		case LeftThigh:
		case RightShin:
		case LeftShin:
		case LeftFoot:
		case RightFoot:
		case RightHand:
		case LeftHand:
		case RightUpperArm:
		case RightLowerArm:
		case LeftUpperArm:
		case LeftLowerArm:
			return limbs_safe;
		default:
			return false;
		}
	};

	// set priority level
	for (auto& point : points) {
		if (is_baim_hitbox(point.hitbox) && point.damage > health)
			point.priority_level += 2;

		if (check_safety(point.hitbox))
			point.priority_level += point.safe;

		if (point.center)
			point.priority_level++;
	}

	std::sort(points.begin(), points.end(), [](CPoint a, CPoint b) {
		return a.priority_level > b.priority_level;
	});

	// erase invalid points
	for (auto it = points.rbegin(); it != points.rend();) {
		if (check_safety(it->hitbox, true) && it->safe < 2)
			it = decltype(it) {
			points.erase(next(it).base())
		};
		else
			it = next(it);
	}

	if (points.empty())
		return CPoint{};

	if (points.size() == 1) // no choice
		return points[0];

	// find the best point
	CPoint best_point{ .damage = -1.f };

	int priority = points[0].priority_level;
	while (priority >= 0) {
		for (auto& point : points) {
			if (point.priority_level != priority)
				break;
			if (point.damage > best_point.damage)
				best_point = point;
		}

		if (priority == 0 || best_point.damage > health)
			return best_point;

		--priority;
	}

	return best_point;

	//switch (scan_mode)
	//{
	//default:
	//case ScanType::SCAN_DEFAULT:
	//{
	//	// in fact this scan mode means we're choosing points by best damage,
	//	// but we're checking first lethal damage in body
	//	CPoint bestPoint{ .damage = -1.f };
	//	for (const auto& p : points)
	//		if (p.damage > bestPoint.damage && is_baim_hitbox(p.hitbox))
	//			bestPoint = p;

	//	if (bestPoint.damage > health)
	//		return bestPoint;

	//	bestPoint.damage = -1.f;

	//	for (const auto& p : points)
	//		if (p.damage > bestPoint.damage)
	//			bestPoint = p;

	//	return bestPoint;
	//};
	//case ScanType::SCAN_PREFER_SAFEPOINT:
	//{
	//	CPoint bestPoint{ .damage = -1.f };

	//	for (const auto& p : points) // layer 1
	//		if (p.damage > bestPoint.damage && p.safe && is_baim_hitbox(p.hitbox))
	//			bestPoint = p;

	//	if (bestPoint.damage > health)
	//		return bestPoint;

	//	bestPoint.damage = -1.f;

	//	for (const auto& p : points) // layer 2
	//		if (p.damage > bestPoint.damage && p.safe)
	//			bestPoint = p;


	//	if (bestPoint.damage != -1.f)
	//		return bestPoint;

	//	bestPoint.damage = -1.f;

	//	for (const auto& p : points) // layer 3
	//		if (p.damage > bestPoint.damage && is_baim_hitbox(p.hitbox))
	//			bestPoint = p;

	//	if (bestPoint.damage > health)
	//		return bestPoint;

	//	bestPoint.damage = -1.f;

	//	for (const auto& p : points) // layer 4
	//		if (p.damage > bestPoint.damage)
	//			bestPoint = p;

	//	return bestPoint;
	//};
	//case ScanType::SCAN_FORCE_SAFEPOINT:
	//{
	//	CPoint bestPoint{ .damage = -1.f };

	//	// force safepoint is easier than prefer safepoint cuz in every scan mode
	//	// we're checking for lethal damage in body first
	//	// than we're checking other hitboxes etc.
	//	// so, here we're checking safe and body first, than safe & head and limbs

	//	for (const auto& p : points)
	//		if (p.damage > bestPoint.damage && p.safe && is_baim_hitbox(p.hitbox))
	//			bestPoint = p;

	//	if (bestPoint.damage > health)
	//		return bestPoint;

	//	bestPoint.damage = -1.f;

	//	for (const auto& p : points)
	//		if (p.damage > bestPoint.damage && p.safe)
	//			bestPoint = p;

	//	return bestPoint;
	//};
	//case ScanType::SCAN_SAFE_BAIM:
	//{
	//	CPoint bestPoint{ .damage = -1.f };

	//	// this is easiest scan mode 'cause we're checking only safe points in body

	//	for (const auto& p : points)
	//		if (p.damage > bestPoint.damage && p.safe && is_baim_hitbox(p.hitbox))
	//			bestPoint = p;

	//	return bestPoint;
	//};
	//}
}

Vector CRagebot::HeadScan(animation* anims, int& hitbox, float min_dmg, int& i) {
	int idx = anims->player->EntIndex();

	memcpy(BoneMatrix, anims->bones, sizeof(matrix[128]));
	memcpy(BoneMatrixInversed, anims->inversed_bones, sizeof(matrix[128]));

	SetAnims(anims, BoneMatrix);
	int health = anims->player->GetHealth();
	if (min_dmg > health)
		min_dmg = health + 1;
	hitbox = 0;

	std::vector<CPoint> points = {};

	for (const auto& Hitbox : GetMultipoints(anims->player, 0, BoneMatrix)) {
		if (!CanHitHitbox(csgo->local->GetEyePosition(), Hitbox.first, anims, 0, anims->unresolved_bones)
			|| !CanHitHitbox(csgo->local->GetEyePosition(), Hitbox.first, anims, 0, BoneMatrixInversed))
			continue;
		i++;
		const auto& damage = g_AutoWall->Think(Hitbox.first, anims->player, HitboxToHitgroup(CSGOHitboxID::Head)).m_damage;

		if (damage > min_dmg)
		{
			points.emplace_back(CPoint{
				.position = Hitbox.first,
				.center = Hitbox.second,
				.hitbox = hitbox,
				.damage = (float)damage,
				.lethal = damage > health,
				.safe = 2
				});
		}
	}

	auto scan_mode = ScanType::SCAN_FORCE_SAFEPOINT;
	auto best_point = GetBestPoint(points, dt_ready ? health / 2.f : health, scan_mode);

	RestorePlayer(anims);
	hitbox = best_point.hitbox;
	anims->safepoints = best_point.safe;
	anims->priority = best_point.priority_level;
	best_damage = best_point.damage;
	return best_point.position;
}

Vector CRagebot::PrimaryScan(animation* anims, int& hitbox, float& simtime, float min_dmg) {
	int idx = anims->player->EntIndex();

	memcpy(BoneMatrix, anims->bones, sizeof(matrix[128]));

	simtime = anims->sim_time;
	SetAnims(anims, BoneMatrix);

	best_damage = -1;

	auto best_point = Vector(0, 0, 0);
	auto health = anims->player->GetHealth();
	if (min_dmg > health)
		min_dmg = health + 1;

	static const vector<int> hitboxes = {
		CSGOHitboxID::Head,
		CSGOHitboxID::Stomach,
		CSGOHitboxID::LeftFoot,
		CSGOHitboxID::RightFoot,
	};
	int i = 0;
	for (auto HitboxID : hitboxes)
	{
		const auto& point = GetPoint(anims->player, HitboxID, BoneMatrix);
		const auto& damage = g_AutoWall->Think(point, anims->player, HitboxToHitgroup((CSGOHitboxID)HitboxID)).m_damage;

		if (damage > best_damage)
		{
			hitbox = HitboxID;
			best_point = point;
			best_damage = damage;
		}
		if (damage < 1.f && i > 0 && best_damage < 1.f)
			break;
		++i;
	}
	RestorePlayer(anims);
	return best_point;
}

Vector CRagebot::FullScan(animation* anims, int& hitbox, float& simtime, float& best_damage, float min_dmg) {
	int idx = anims->player->EntIndex();

	memcpy(BoneMatrix, anims->bones, sizeof(matrix[128]));
	memcpy(BoneMatrixInversed, anims->inversed_bones, sizeof(matrix[128]));

	simtime = anims->sim_time;
	best_damage = -1;
	SetAnims(anims, BoneMatrix);

	int health = anims->player->GetHealth();
	if (min_dmg > health)
		min_dmg = health + 1;
	auto hitboxes = GetHitboxesToScan(anims->player);

	std::vector<CPoint> points;
	for (auto HitboxID : hitboxes)
	{
		for (const auto& Hitbox : GetMultipoints(anims->player, HitboxID, BoneMatrix)) {
			const auto& damage = g_AutoWall->Think(Hitbox.first, anims->player, HitboxToHitgroup((CSGOHitboxID)HitboxID)).m_damage;
			if (damage > min_dmg)
				points.emplace_back(CPoint{
					.position = Hitbox.first,
					.center = Hitbox.second,
					.hitbox = HitboxID,
					.damage = (float)damage,
					.lethal = damage > health,
					.safe = (int)CanHitHitbox(csgo->local->GetEyePosition(), Hitbox.first, anims, HitboxID, anims->unresolved_bones)
						+ (int)CanHitHitbox(csgo->local->GetEyePosition(), Hitbox.first, anims, HitboxID, BoneMatrixInversed)
					});
		}
	}

	const auto& scan_mode = ScanMode(anims->player);
	const auto& best_point = GetBestPoint(points, dt_ready ? health / 2.f : health, scan_mode);

	RestorePlayer(anims);
	hitbox = best_point.hitbox;
	anims->safepoints = best_point.safe;
	anims->priority = best_point.priority_level;
	best_damage = best_point.damage;
	return best_point.position;
}
Vector CRagebot::GetAimVector(IBasePlayer* pTarget, float& simtime, animation*& best_anims, int& hitbox)
{
	float m_damage = 0.f;
	if (csgo->weapon->IsZeus()) {
		m_damage = 100.f;
	}
	else {
		m_damage = g_Binds[bind_override_dmg].active ? current_settings.mindamage_override
			: current_settings.mindamage;
	}

	const auto& latest_animation = g_Animfix->get_latest_animation(pTarget);
	auto record = latest_animation;
	if (!record || !record->player)
		return Vector(0, 0, 0);

	BackupPlayer(record);

	const auto& oldest_animation = g_Animfix->get_oldest_animation(pTarget);

	Vector latest_origin = Vector(0, 0, 0);
	float best_damage_0 = -1.f, best_damage_1 = -1.f;

	record = latest_animation;
	if (record)
	{
		latest_origin = record->origin;
		best_damage = -1.f;
		Vector full = PrimaryScan(record, hitbox, simtime, m_damage);
		if (full != Vector(0, 0, 0))
			best_damage_0 = best_damage;
	}

	record = oldest_animation;

	if (record && record->origin.DistTo(latest_animation->origin) > 25.f
		&& record->resolver_mode == latest_animation->resolver_mode) // stupid fixes, but work sometimes
	{
		best_damage = -1.f;
		Vector full = PrimaryScan(record, hitbox, simtime, m_damage);
		if (full != Vector(0, 0, 0))
			best_damage_1 = best_damage;
	}

	if (best_damage_0 >= best_damage_1)
		record = latest_animation;
	else
		record = oldest_animation;
	
	if (!(best_damage_0 > 0.f || best_damage_1 > 0.f) && !dt_ready) {
		auto valid_animations = g_Animfix->get_valid_animations(pTarget);
		int i = 0;
		if (current_settings.hitscan & 1 && !g_Binds[bind_baim].active) {
			best_damage = -1.f;
			Vector best_point = Vector(0, 0, 0);
			for (const auto& rec : valid_animations) {
				Vector point = HeadScan(rec, hitbox, m_damage, i);
				if (point != Vector(0, 0, 0)) {
					best_anims = rec;
					simtime = rec->sim_time;
					best_point = point;
				}
				if (i > 1)
					break;
			}
			if (best_damage != -1.f)
				return best_point;
		}
	}

	if (record)
	{
		best_anims = record;
		Vector full_scan = FullScan(record, hitbox, simtime, best_damage, m_damage);
		int health = record->player->GetHealth();

		simtime = record->sim_time;
		best_anims = record;
		return full_scan;
	}

	
	return Vector(0, 0, 0);
}

static std::vector<std::tuple<float, float, float>> precomputed_seeds = {};

typedef void(*RandomSeed_t)(UINT);
RandomSeed_t m_RandomSeed = 0;
void random_seed(uint32_t seed) {
	if (m_RandomSeed == nullptr)
		m_RandomSeed = (RandomSeed_t)GetProcAddress(GetModuleHandle(str("vstdlib.dll")), str("RandomSeed"));
	m_RandomSeed(seed);
}

typedef float(*RandomFloat_t)(float, float);
RandomFloat_t m_RandomFloat;
float random_float(float flLow, float flHigh)
{
	if (m_RandomFloat == nullptr)
		m_RandomFloat = (RandomFloat_t)GetProcAddress(GetModuleHandle(str("vstdlib.dll")), str("RandomFloat"));

	return m_RandomFloat(flLow, flHigh);
}

static const int total_seeds = 255;

void build_seed_table()
{
	if (!precomputed_seeds.empty())
		return;

	for (auto i = 0; i < total_seeds; i++) {
		random_seed(i + 1);

		const auto pi_seed = random_float(0.f, PI * 2);

		precomputed_seeds.emplace_back(random_float(0.f, 1.f),
			sin(pi_seed), cos(pi_seed));
	}
}

static auto get_angle = [](Vector base, Vector start, Vector end) {
	auto a = base - start;
	Vector b = base - end;

	//if (a.y < 0.f)
	//	return float(PI * 2.f) - (acos((a.x * b.x + a.y * b.y + a.z * b.z) / (a.Length() * b.Length())));

	return acos((a.x * b.x + a.y * b.y + a.z * b.z) / (a.Length() * b.Length()));
};

bool HitTraces(float& final_chance, animation* _animation, const Vector position, const float chance, int box, matrix* bones)
{
	build_seed_table();

	const auto weapon = csgo->weapon;

	if (!weapon)
		return false;

	const auto info = weapon->GetCSWpnData();

	if (!info)
		return false;

	const auto studio_model = interfaces.models.model_info->GetStudioModel(_animation->player->GetModel());

	if (!studio_model)
		return false;

	// setup calculation parameters.
	const auto id = weapon->GetItemDefinitionIndex();
	const auto round_acc = [](const float accuracy) { return roundf(accuracy * 1000.f) / 1000.f; };
	const auto sniper = weapon->isSniper();
	const auto crouched = csgo->local->GetFlags() & FL_DUCKING;

	// calculate inaccuracy.
	const auto weapon_inaccuracy = weapon->GetInaccuracy();
	const auto weap_spread = weapon->GetSpread();

	// calculate start and angle.
	auto start = csgo->local->GetEyePosition();
	const auto aim_angle = Math::CalculateAngle(start, position);
	Vector forward, right, up;	
	Math::AngleVectors(aim_angle, forward, right, up);

	// keep track of all traces that hit the enemy.
	auto current = 0;

	// setup calculation parameters.
	Vector total_spread, spread_angle, end;
	float inaccuracy, spread_x, spread_y;
	std::tuple<float, float, float>* seed;

	for (auto i = 0u; i < total_seeds; i++)  // NOLINT(modernize-loop-convert)
	{
		// get seed.
		seed = &precomputed_seeds[i];

		// calculate spread.
		inaccuracy = std::get<0>(*seed) * weapon_inaccuracy;
		spread_x = std::get<2>(*seed) * inaccuracy;
		spread_y = std::get<1>(*seed) * inaccuracy;
		total_spread = (forward + right * spread_x + up * spread_y).NormalizeInPlace();

		// calculate angle with spread applied.
		Math::VectorAngles(total_spread, spread_angle);

		// calculate end point of trace.
		Math::AngleVectors(spread_angle, end);
		end = start + end.NormalizeInPlace() * info->m_flRange;

		// did we hit the hitbox?

		if (CanHitHitbox(start, end, _animation, box, bones))
			current++;

		//// abort if hitchance is already sufficent.
		//if ((float)current / (float)total_seeds >= chance) {
		//	final_chance = (float)current / (float)total_seeds;
		//	return true;
		//}

		// abort if we can no longer reach hitchance.
		if ((float)(current + total_seeds - i) / (float)(total_seeds) < chance)
			return false;
	}

	final_chance = (float)current / (float)total_seeds;

	return (float)current / (float)total_seeds >= chance;
}
bool CRagebot::Hitchance(Vector Aimpoint, animation* best, int& hitbox)
{
	float chance = current_settings.hitchance;
	const bool& scoped = csgo->weapon->isSniper() && !csgo->local->IsScoped();
	if (csgo->weapon->IsZeus())
		chance = vars.ragebot.zeuschance / 100.f;
	else
		chance /= 100.f;

	if (g_Misc->dt_bullets == 1) {
		chance = current_settings.doubletap_hc / 100.f;
		if (scoped)
			chance /= 1.5f;
	}
	else
	{
		if (scoped)
			chance /= 2.f;
	}

	chance = clamp(chance, 0.f, 1.f);

	int idx = best->player->EntIndex();

	if (/*g_Binds[bind_force_safepoint].active ||*/ best->safepoints == 2) {
		float final_hitchance[3] = { 0.f };
		bool ret = HitTraces(final_hitchance[0], best, Aimpoint, chance, hitbox, best->bones)
			&& HitTraces(final_hitchance[1], best, Aimpoint, chance, hitbox, best->inversed_bones)
			&& HitTraces(final_hitchance[2], best, Aimpoint, chance, hitbox, best->unresolved_bones);
		best_hitchance = (final_hitchance[0] + final_hitchance[1] + final_hitchance[2]) / 3.f;
		return ret;
	}
	else {
		return HitTraces(best_hitchance, best, Aimpoint, chance, hitbox, best->bones);
	}
}

Vector CRagebot::GetVisualHitbox(IBasePlayer* ent, int ihitbox)
{
	if (ihitbox < 0 || ihitbox > 19) return Vector(0, 0, 0);

	if (!ent) return Vector(0, 0, 0);

	const auto model = ent->GetModel();

	if (!model)
		return Vector(0, 0, 0);

	auto pStudioHdr = interfaces.models.model_info->GetStudioModel(model);

	if (!pStudioHdr)
		return Vector(0, 0, 0);

	auto hitbox = pStudioHdr->pHitbox(ihitbox, 0);

	if (!hitbox)
		return Vector(0, 0, 0);

	Vector min, max;
	Math::VectorTransform(hitbox->bbmin, ent->GetBoneCache()[hitbox->bone], min);
	Math::VectorTransform(hitbox->bbmax, ent->GetBoneCache()[hitbox->bone], max);

	auto center = (min + max) / 2.f;

	return center;
}

bool CRagebot::IsAbleToShoot(float tick_shift)
{
	if (!csgo->local || !csgo->local->GetWeapon())
		return false;

	if (csgo->game_rules->IsFreezeTime())
		return false;

	if (csgo->cmd->weaponselect != 0)
		return false;

	if (csgo->local->GetFlags() & 0x40)
		return false;

	if (csgo->local->WaitForNoAttack())
		return false;

	if (csgo->local->IsDefusing())
		return false;

	if (csgo->weapon->InReload())
		return false;

	if (csgo->local->GetPlayerState() > 0)
		return false;

	if (csgo->weapon->GetAmmo(false) <= 0 && !csgo->weapon->IsKnife() && !csgo->weapon->IsZeus())
		return false;

	if (csgo->weapon->GetItemDefinitionIndex() == weapon_revolver)
		return m_revolver_fire;

	auto time = TICKS_TO_TIME(csgo->fixed_tickbase - tick_shift);

	if (csgo->local->m_flNextAttack() > time
		|| csgo->weapon->NextPrimaryAttack() > time)
		return false;

	return true;
}

float CRagebot::LerpTime() {
	static auto cl_interp = interfaces.cvars->FindVar(hs::cl_interp.s().c_str());
	static auto cl_updaterate = interfaces.cvars->FindVar(hs::cl_updaterate.s().c_str());
	static auto cl_interp_ratio = interfaces.cvars->FindVar(hs::cl_interp_ratio.s().c_str());

	const auto a2 = cl_updaterate->GetFloat();
	const auto a1 = cl_interp->GetFloat();
	const auto v2 = cl_interp_ratio->GetFloat() / a2;

	return fmaxf(a1, v2);
};

void CRagebot::DropTarget()
{
	ShouldWork = false;
	target_index = -1;
	best_distance = FLT_MAX;
	current_aim_position = Vector();
	csgo->should_stop_slide = false;
	csgo->last_forced_tickcount = -1;
	csgo->should_stop = false;
	g_AutoWall->reset();
}

void CockRevolver() {

	if (!vars.ragebot.enable)
		return;

	if (!csgo->local || !csgo->local->isAlive())
		return;

	if (csgo->weapon->GetItemDefinitionIndex() != WEAPON_REVOLVER)
		return;

	if (csgo->weapon->GetAmmo(false) <= 0)
		return;

	if (csgo->local->m_flNextAttack() > TICKS_TO_TIME(csgo->fixed_tickbase))
		return;

	static auto last_checked = 0;
	static auto last_spawn_time = 0.f;
	static auto tick_cocked = 0;
	static auto tick_strip = 0;
	auto time = TICKS_TO_TIME(csgo->fixed_tickbase);


	const auto max_ticks = TIME_TO_TICKS(.25f) - 1;
	const auto tick_base = TIME_TO_TICKS(time);

	if (csgo->local->GetSpawnTime() != last_spawn_time) {
		tick_cocked = tick_base;
		tick_strip = tick_base - max_ticks - 1;
		last_spawn_time = csgo->local->GetSpawnTime();
	}

	if (csgo->weapon->NextPrimaryAttack() > time) {
		csgo->cmd->buttons &= ~IN_ATTACK;
		g_Ragebot->m_revolver_fire = false;
		return;
	}

	if (last_checked == tick_base)
		return;

	last_checked = tick_base;
	g_Ragebot->m_revolver_fire = false;

	if (tick_base - tick_strip > 2 && tick_base - tick_strip < 14)
		g_Ragebot->m_revolver_fire = true;

	if (csgo->cmd->buttons & IN_ATTACK && g_Ragebot->m_revolver_fire)
		return;

	csgo->cmd->buttons |= IN_ATTACK;

	if (csgo->weapon->NextSecondaryAttack() >= time)
		csgo->cmd->buttons |= IN_ATTACK2;

	if (tick_base - tick_cocked > max_ticks * 2 + 1) {
		tick_cocked = tick_base;
		tick_strip = tick_base - max_ticks - 1;
	}

	const auto cock_limit = tick_base - tick_cocked >= max_ticks;
	const auto after_strip = tick_base - tick_strip <= max_ticks;

	if (cock_limit || after_strip) {
		tick_cocked = tick_base;
		csgo->cmd->buttons &= ~IN_ATTACK;

		if (cock_limit)
			tick_strip = tick_base;
	}
}

string HitboxToString(int id)
{
	switch (id)
	{
	case 0: return hs::head.s();  break;
	case 1: return hs::neck.s(); break;
	case 2: return hs::pelvis.s(); break;
	case 3: return hs::stomach.s(); break;
	case 4: return hs::lower_chest.s(); break;
	case 5: return hs::chest.s(); break;
	case 6: return hs::upper_chest.s(); break;
	case 7: return hs::right_thigh.s(); break;
	case 8: return hs::left_thigh.s(); break;
	case 9: return hs::right_leg.s(); break;
	case 10: return hs::left_leg.s(); break;
	case 11: return hs::left_foot.s(); break;
	case 12: return hs::right_foot.s(); break;
	case 13: return hs::right_hand.s(); break;
	case 14: return hs::left_hand.s(); break;
	case 15: return hs::right_upper_arm.s(); break;
	case 16: return hs::right_lower_arm.s(); break;
	case 17: return hs::left_upper_arm.s(); break;
	case 18: return hs::left_lower_arm.s(); break;
	}
}

string ShotSnapshot::get_info() {
	string ret;

	ret += str("hitbox: ") + hitbox_where_shot;
	ret += str("; damage: ") + std::to_string(intended_damage);
	
	ret += str("; hitchance: ") + std::to_string((int)(hitchance * 100.f));
	ret += str("; resolver: ") + this->resolver_mode + str(" | ") + std::to_string(g_Resolver->ResolverInfo[this->entity->GetIndex()].Index);

	if (record.safepoints == 2)
		ret += str("; safepoints");

	if (backtrack > 0)
		ret += str("; backtrack: ") + std::to_string(backtrack);

	ret += str("; priority: ") + std::to_string(record.priority);
	return ret;
}

bool CRagebot::FastStop() {
	auto wpn_info = csgo->weapon->GetCSWpnData();
	if (!wpn_info)
		return false;

	auto get_standing_accuracy = [&]() -> const float
	{
		const auto max_speed = csgo->weapon->GetZoomLevel() > 0 ? wpn_info->m_flMaxSpeedAlt : wpn_info->m_flMaxSpeed;
		return max_speed / 3.f;
	};
	
	float speed = csgo->local->GetVelocity().Length2D();
	if (speed <= get_standing_accuracy())
		return true;

	float max_speed = (csgo->weapon->GetZoomLevel() == 0 ? wpn_info->m_flMaxSpeed : wpn_info->m_flMaxSpeedAlt) * 0.1f;
	if (speed > max_speed) {
		csgo->should_stop_slide = false;
	}
	else {
		csgo->should_stop_slide = true;
		return true;
	}

	Vector direction;
	Math::VectorAngles(csgo->local->GetVelocity(), direction);
	direction.y = csgo->original.y - direction.y;
	Vector forward;
	Math::AngleVectors(direction, forward);
	Vector negated_direction = forward * -speed;
	csgo->cmd->forwardmove = negated_direction.x;
	csgo->cmd->sidemove = negated_direction.y;
	return !(current_settings.quickstop_options & 2);
}

int CRagebot::GetTicksToShoot() {
	if (IsAbleToShoot())
		return -1;
	auto flServerTime = TICKS_TO_TIME(csgo->fixed_tickbase);
	auto flNextPrimaryAttack = csgo->local->GetWeapon()->NextPrimaryAttack();

	return TIME_TO_TICKS(fabsf(flNextPrimaryAttack - flServerTime));
}

int CRagebot::GetTicksToStop() {
	static auto predict_velocity = [](Vector* velocity)
	{
		float speed = velocity->Length2D();
		static auto sv_friction = interfaces.cvars->FindVar(hs::sv_friction.s().c_str());
		static auto sv_stopspeed = interfaces.cvars->FindVar(hs::sv_stopspeed.s().c_str());

		if (speed >= 1.f)
		{
			float friction = sv_friction->GetFloat();
			float stop_speed = std::max< float >(speed, sv_stopspeed->GetFloat());
			float time = std::max< float >(interfaces.global_vars->interval_per_tick, interfaces.global_vars->frametime);
			*velocity *= std::max< float >(0.f, speed - friction * stop_speed * time / speed);
		}
	};
	Vector vel = csgo->vecUnpredictedVel;
	int ticks_to_stop = 0;
	for (;;)
	{
		if (vel.Length2D() < 1.f)
			break;
		predict_velocity(&vel);
		ticks_to_stop++;
	}
	return ticks_to_stop;
}

void CRagebot::DoQuickStop()
{
	if (!vars.ragebot.enable)
		return;

	if (this->do_auto_stop) {
		if (!FastStop())
			return;

		if (csgo->weapon->GetItemDefinitionIndex() != WEAPON_SSG08
			|| csgo->weapon->GetItemDefinitionIndex() != WEAPON_AWP)
			csgo->should_stop_slide = true;

		this->do_auto_stop = false;
 	}
}

bool CRagebot::HoldFiringAnimation() {
	return (csgo->weapon && !IsAbleToShoot() &&
		fabsf(csgo->weapon->LastShotTime() - TICKS_TO_TIME(csgo->fixed_tickbase)) < 0.2f);
}

void CRagebot::DrawCapsule(IBasePlayer* player, matrix* bones, color_t clr) {
	studiohdr_t* pStudioModel = interfaces.models.model_info->GetStudioModel(player->GetModel());
	if (!pStudioModel)
		return;
	mstudiohitboxset_t* pHitboxSet = pStudioModel->pHitboxSet(0);
	if (!pHitboxSet)
		return;

	for (int i = 0; i < pHitboxSet->numhitboxes; i++)
	{
		mstudiobbox_t* pHitbox = pHitboxSet->pHitbox(i);
		if (!pHitbox)
			continue;

		Vector vMin, vMax;
		Math::VectorTransform(pHitbox->bbmin, bones[pHitbox->bone], vMin);
		Math::VectorTransform(pHitbox->bbmax, bones[pHitbox->bone], vMax);

		int r = 255,
			g = 255,
			b = 255,
			a = 255;

		if (pHitbox->radius != -1)
			interfaces.debug_overlay->add_capsule_overlay(vMin, vMax, pHitbox->radius, clr.get_red(), clr.get_green(), clr.get_blue(), clr.get_alpha(), 5.f, 0, 1);
	}
}

void CRagebot::Run()
{
	auto weapon = csgo->weapon;

	if (!weapon->IsGun())
		return;

	if (csgo->weapon->IsZeus() && !vars.ragebot.zeusbot)
		return;

	this->shot = false;

	int curhitbox;
	animation* best_anims = nullptr;
	int hitbox = -1;

	float simtime = 0;
	animation* anims = nullptr;
	int current_hitbox;
	shot = false;
	csgo->should_stop_slide = false;

	bool in_air = !(csgo->local->GetFlags() & FL_ONGROUND);
	bool is_revolver = weapon->GetItemDefinitionIndex() == WEAPON_REVOLVER;
	CockRevolver();

	Vector viewangles;
	interfaces.engine->GetViewAngles(viewangles);

	bool is_able_to_shoot = IsAbleToShoot();

	current_settings = CurrentSettings();

	dt_ready = g_Binds[bind_double_tap].active && g_Misc->dt_bullets <= 1 && csgo->weapon->isAutoSniper();
	
	for (auto i = 1; i <= interfaces.global_vars->maxClients; ++i) {
		auto pEntity = interfaces.ent_list->GetClientEntity(i);
		if (!pEntity || pEntity == csgo->local || !pEntity->IsPlayer()) {
			g_Resolver->ClearHitInfo(i);
			continue;
		}
		if (!pEntity->isAlive() || pEntity->GetHealth() <= 0 || pEntity->GetTeam() == csgo->local->GetTeam()) {
			csgo->actual_misses[pEntity->GetIndex()] = 0;
			csgo->imaginary_misses[pEntity->GetIndex()] = 0;
			continue;
		}
		if (pEntity->IsDormant() || pEntity->HasGunGameImmunity())
			continue;

		target_lethal = false;

		Vector aim_position = GetAimVector(pEntity, simtime, anims, current_hitbox);

		if (!anims)
			continue;

		float distance = csgo->local->GetAbsOrigin().DistTo(pEntity->GetAbsOrigin());
		if (distance < best_distance
			&& anims->player == pEntity && aim_position != Vector(0, 0, 0)
			&& Math::GetFov(viewangles, Math::CalculateAngle(csgo->eyepos, aim_position)) <= vars.ragebot.fov)
		{
			best_distance = distance;
			target_index = i;
			current_aim_position = aim_position;
			current_aim_simulationtime = simtime;
			best_anims = anims;
			hitbox = current_hitbox;
		}
	}

	if (hitbox != -1 && target_index != -1 && best_anims && current_aim_position != Vector(0, 0, 0))
	{
		ShouldWork = true;
		bool can_shoot_if_fakeduck = true;

		if (g_Binds[bind_fake_duck].active) {
			can_shoot_if_fakeduck = csgo->local->GetFlags() & FL_ONGROUND && (csgo->duck_amount > 0.9f || csgo->duck_amount < 0.1f);
		}

		bool hitchance = Hitchance(current_aim_position, best_anims, hitbox);

		if (vars.ragebot.autoscope == 1) {
			if (weapon->isSniper() && csgo->weapon->GetZoomLevel() == 0)
			{
				csgo->cmd->buttons |= IN_ATTACK2;
				return;
			}
		}
		else if (vars.ragebot.autoscope == 2) {
			if (weapon->isSniper() && csgo->weapon->GetZoomLevel() == 0 && !hitchance)
			{
				csgo->cmd->buttons |= IN_ATTACK2;
				return;
			}
		}

		csgo->should_stop_slide = false;

		this->force_accuracy = !current_settings.quickstop;

		if (csgo->local->GetFlags() & FL_ONGROUND && !g_Binds[bind_slow_walk].active) {
			if (!csgo->weapon->IsZeus()) {
				bool should_stop = GetTicksToShoot() <= GetTicksToStop()
					|| (current_settings.quickstop_options & 1) && !is_able_to_shoot;
				if (should_stop && current_settings.quickstop)
				{
					if (!csgo->should_stop_slide)
						force_accuracy = FastStop();
					csgo->should_stop_slide = true;
				}
				else
					csgo->should_stop_slide = false;
			}
		}

		if (!(csgo->local->GetFlags() & FL_ONGROUND))
			force_accuracy = true;

		if (!current_settings.quickstop
			|| g_Binds[bind_slow_walk].active
			|| csgo->weapon->IsZeus()
			|| csgo->weapon->GetItemDefinitionIndex() == weapon_revolver)
			force_accuracy = true;

		if (vars.ragebot.autoshoot) {

			if (hitchance && can_shoot_if_fakeduck && is_able_to_shoot)
			{
				if (!csgo->fake_duck) {
					if (!csgo->last_sendpacket || g_Binds[bind_double_tap].active)
						csgo->send_packet = true;
					csgo->max_fakelag_choke = CanExploit() ? 1 : 2;
				}
				if (force_accuracy)
					csgo->cmd->buttons |= IN_ATTACK;
			}
		}
		if (hitchance && can_shoot_if_fakeduck && is_able_to_shoot)
		{
			if (csgo->cmd->buttons & IN_ATTACK) {
				csgo->last_forced_tickcount = csgo->cmd->tick_count;

				csgo->cmd->tick_count = TIME_TO_TICKS(best_anims->sim_time + LerpTime());
				csgo->cmd->viewangles = Math::CalculateAngle(csgo->local->GetEyePosition(), current_aim_position);
				csgo->cmd->viewangles -= csgo->local->GetPunchAngle() * interfaces.cvars->FindVar(str("weapon_recoil_scale"))->GetFloat();

				if (!vars.misc.antiuntrusted) {
					csgo->cmd->viewangles.y += 180.f;
					csgo->cmd->viewangles.x += 180.f - csgo->cmd->viewangles.x * 2.f;
				}

				ShotSnapshot* snapshot = new ShotSnapshot();
				snapshot->entity = best_anims->player;
				snapshot->hitbox_where_shot = HitboxToString(hitbox);
				snapshot->resolver_mode = ResolverMode[best_anims->player->GetIndex()];
				snapshot->time = TICKS_TO_TIME(csgo->fixed_tickbase);
				snapshot->first_processed_time = 0.f;
				snapshot->bullet_impact = false;
				snapshot->weapon_fire = false;
				snapshot->damage = -1;
				snapshot->resolver_info = g_Resolver->ResolverInfo[best_anims->player->GetIndex()];
				snapshot->intended_damage = best_damage;
				snapshot->start = csgo->local->GetEyePosition();
				snapshot->hitgroup_hit = -1;
				snapshot->backtrack = TIME_TO_TICKS(fabsf(best_anims->player->GetSimulationTime() - best_anims->sim_time));
				snapshot->eyeangles = Math::normalize(best_anims->player->GetEyeAngles());
				snapshot->hitbox = hitbox;
				snapshot->player_name = best_anims->player->GetName();
				snapshot->intended_position = current_aim_position;
				snapshot->hitchance = best_hitchance;
				snapshot->record = *best_anims;

				current_event_push(str("player_index"), best_anims->player->GetIndex());
				current_event_push(str("hitbox"), hitbox);
				current_event_push(str("resolver_mode"), snapshot->resolver_mode);
				current_event_push(str("safepoint"), best_anims->safepoints);
				current_event_push(str("priority"), best_anims->priority);
				current_event_push(str("hitchance"), best_hitchance);
				current_event_push(str("backtrack"), snapshot->backtrack);

				run_callback("ragebot_fire");

				current_event_clear();

				if (vars.visuals.eventlog & 2) {
					interfaces.cvars->ConsoleColorPrintf(vars.visuals.eventlog_color, hs::prefix.s().c_str());
					interfaces.cvars->ConsoleColorPrintf(color_t(255, 255, 255), hs::_s_n.s().c_str(),
						(str("Fired shot ") + snapshot->entity->GetName() + str(" : ") + snapshot->get_info()).c_str());
				}
				
				shot_snapshots.emplace_back(*snapshot);
				delete snapshot;
				shot = true;
				if (vars.visuals.chams[enemy_ragebot_shot].enable) {

					// вот эту хуйню расскоменчивать когда надо тест анимфикса + сейфпоинтов, чтоб видеть куда рейдж стреляет

					//DrawCapsule(best_anims->player, best_anims->unresolved_bones, color_t(255, 255, 255, 127));
					//DrawCapsule(best_anims->player, best_anims->right_side_bone, color_t(255, 35, 35, 127));
					//DrawCapsule(best_anims->player, best_anims->left_side_bone, color_t(35, 255, 35, 127));
					g_Chams->AddHitmatrix(best_anims);
				}
				if (!vars.ragebot.silent)
					interfaces.engine->SetViewAngles(csgo->cmd->viewangles);
			}
		}
	}
	if (is_able_to_shoot && csgo->cmd->buttons & IN_ATTACK && !is_revolver)
		shot = true;
}