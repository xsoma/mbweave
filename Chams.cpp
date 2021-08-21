#include "Hooks.h"
#include "key_values.h"
#include "Features.h"

inline void SetMatrixOrigin(Vector pos, matrix& m)
{
	m[0][3] = pos.x;
	m[1][3] = pos.y;
	m[2][3] = pos.z;
}

inline Vector GetMatrixOrigin(const matrix& src)
{
	return { src[0][3], src[1][3], src[2][3] };
}

IMaterial* CreateMaterial(bool lit, const std::string& material_data)
{
	static auto created = 0;
	std::string type = lit ? str("VertexLitGeneric") : str("UnlitGeneric");

	auto matname = str("material_number_") + std::to_string(created);
	++created;

	auto keyValues = new KeyValues(matname.c_str());
	static auto key_values_address = csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		str("55 8B EC 51 33 C0 C7 45"));

	using KeyValuesFn = void(__thiscall*)(void*, const char*);
	reinterpret_cast <KeyValuesFn> (key_values_address)(keyValues, type.c_str());

	static auto load_from_buffer_address = csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		str("55 8B EC 83 E4 F8 83 EC 34 53 8B 5D 0C 89"));
	using LoadFromBufferFn = void(__thiscall*)(void*, const char*, const char*, void*, const char*, void*, void*);

	reinterpret_cast <LoadFromBufferFn> (load_from_buffer_address)(keyValues, matname.c_str(), material_data.c_str(), NULL, NULL, NULL, NULL);

	auto material = interfaces.material_system->CreateMaterial(matname.c_str(), keyValues);
	material->IncrementReferenceCount();

	return material;
}

bool GetBacktrackMaxtrix(IBasePlayer* player, matrix* out)
{
	if (vars.ragebot.enable || vars.legitbot.enable) {
		const auto& first_invalid = g_Animfix->get_oldest_animation(player);

		if (!first_invalid)
			return false;

		if ((first_invalid->origin - player->GetOrigin()).Length2D() < 1.f)
			return false;

		if (first_invalid->dormant)
			return false;

		memcpy(out, first_invalid->unresolved_bones, sizeof(matrix[128]));
		return true;
	}
	return false;
}

IMaterial* CChams::GetMaterial(int index)
{
	switch (index)
	{
	case 0:
		return this->Default;
		break;
	case 1:
		return this->Flat;
		break;
	case 2:
		return this->Glass;
		break;
	case 3:
		return this->Metallic;
		break;
	default:
		return this->Default;
		break;
	}
}

bool CChams::DrawChams(const chams_t& chams_data, matrix* mat, bool ignore_z,
	DrawModelExecuteFn original,
	void* thisptr,
	void* ctx,
	const DrawModelState_t& state,
	const ModelRenderInfo_t& info
)
{
	if (!chams_data.enable
		|| mat == nullptr)
		return false;

	auto IsNullMaterial = [&]() {
		if (Default == nullptr ||
			Flat == nullptr ||
			Glass == nullptr ||
			Metallic == nullptr ||
			GlowFade == nullptr ||
			GlowLine == nullptr ||
			Wireframe == nullptr)
			return true;
		return false;
	};

	if (IsNullMaterial())
		return false;

	float BackupColor[3];
	interfaces.render_view->GetColorModulation(BackupColor);
	
	IMaterial* BaseMaterial = GetMaterial(chams_data.material);
	
	if (BaseMaterial)
	{
		float CurrentColor[3] = {
			chams_data.material_color[0] / 255.f,
			chams_data.material_color[1] / 255.f,
			chams_data.material_color[2] / 255.f
		};
		interfaces.render_view->SetColorModulation(CurrentColor);
		BaseMaterial->AlphaModulate(chams_data.material_color[3] / 255.f);

		BaseMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignore_z);

		switch (chams_data.material)
		{
			// glass:
		case 2:
		{
			auto var = BaseMaterial->FindVar(hs::envmaptint.s().c_str(), nullptr);
			var->SetVecValue(
				(chams_data.glass_color[0] / 255.f) * ((chams_data.glass_color[3] / 255.f)),
				(chams_data.glass_color[1] / 255.f) * ((chams_data.glass_color[3] / 255.f)),
				(chams_data.glass_color[2] / 255.f) * ((chams_data.glass_color[3] / 255.f))
			);
		}
		break;
		// metallic: 
		case 3:
		{
			auto var = BaseMaterial->FindVar(hs::phongtint.s().c_str(), nullptr);
			var->SetVecValue(
				(chams_data.metallic_color[0] / 255.f) * ((chams_data.metallic_color[3] / 255.f)),
				(chams_data.metallic_color[1] / 255.f) * ((chams_data.metallic_color[3] / 255.f)),
				(chams_data.metallic_color[2] / 255.f) * ((chams_data.metallic_color[3] / 255.f))
			);

			float phong_exp = clamp(((chams_data.phong_amount / 10.f) - 1.f), 0.f, 10.f);
			{
				auto phong_exponent = BaseMaterial->FindVar(hs::phongexponent.s().c_str(), nullptr);
				phong_exponent->SetVecValue(10.f - phong_exp);

				auto phong_boost = BaseMaterial->FindVar(hs::phongboost.s().c_str(), nullptr);
				phong_boost->SetVecValue(chams_data.phong_amount / 100.f);
			}

			float rim_amt = chams_data.rim_amount;
			{
				auto rim_exponent = BaseMaterial->FindVar(hs::rimlightexponent.s().c_str(), nullptr);
				rim_exponent->SetVecValue(rim_amt);

				auto rim_boost = BaseMaterial->FindVar(hs::rimlightboost.s().c_str(), nullptr);
				rim_boost->SetVecValue(rim_amt);
			}
		}
		break;
		}
		interfaces.models.model_render->ForcedMaterialOverride(BaseMaterial);
		original(thisptr, ctx, state, info, mat);
		interfaces.models.model_render->ForcedMaterialOverride(nullptr);
	}

	// Glow fade
	if (chams_data.overlay & 1)
	{
		IMaterial* MaterialGlow = this->GlowFade;
		if (MaterialGlow)
		{
			MaterialGlow->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignore_z);

			auto var = MaterialGlow->FindVar(hs::envmaptint.s().c_str(), nullptr);
			var->SetVecValue(
				chams_data.glow_color[0][0] / 255.f,
				chams_data.glow_color[0][1] / 255.f,
				chams_data.glow_color[0][2] / 255.f);

			MaterialGlow->AlphaModulate(chams_data.glow_color[0][3] / 255.f);

			interfaces.models.model_render->ForcedMaterialOverride(MaterialGlow);
			original(thisptr, ctx, state, info, mat);
			interfaces.models.model_render->ForcedMaterialOverride(nullptr);
		}
	}
	// Glow line
	if (chams_data.overlay & 2)
	{
		IMaterial* MaterialGlow = this->GlowLine;
		if (MaterialGlow) {
			MaterialGlow->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignore_z);

			auto var = MaterialGlow->FindVar(hs::envmaptint.s().c_str(), nullptr);
			var->SetVecValue(
				chams_data.glow_color[1][0] / 255.f,
				chams_data.glow_color[1][1] / 255.f,
				chams_data.glow_color[1][2] / 255.f);

			MaterialGlow->AlphaModulate(chams_data.glow_color[1][3] / 255.f);

			interfaces.models.model_render->ForcedMaterialOverride(MaterialGlow);
			original(thisptr, ctx, state, info, mat);
			interfaces.models.model_render->ForcedMaterialOverride(nullptr);
		}
	}
	// Wireframe
	if (chams_data.overlay & 4)
	{
		IMaterial* MaterialWireframe = this->Wireframe;
		if (MaterialWireframe) {
			MaterialWireframe->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignore_z);

			float CurrentColor[3] = {
				chams_data.wireframe_color[0] / 255.f,
				chams_data.wireframe_color[1] / 255.f,
				chams_data.wireframe_color[2] / 255.f
			};
			interfaces.render_view->SetColorModulation(CurrentColor);
			MaterialWireframe->AlphaModulate(chams_data.wireframe_color[3] / 255.f);

			interfaces.models.model_render->ForcedMaterialOverride(MaterialWireframe);
			original(thisptr, ctx, state, info, mat);
			interfaces.models.model_render->ForcedMaterialOverride(nullptr);
		}
	}

	interfaces.models.model_render->ForcedMaterialOverride(nullptr);
	interfaces.render_view->SetColorModulation(BackupColor);
	return true;
}

void CChams::AddHitmatrix(animation* record) {
	auto& hit = m_Hitmatrix.emplace_back();
	auto player = record->player;

	int idx = player->EntIndex();

	auto curBones = record->bones;
	if (!curBones)
		return;

	std::memcpy(hit.pBoneToWorld, curBones, player->GetBoneCache().Count() * sizeof(matrix));
	hit.time = interfaces.global_vars->realtime + 4.f;

	int m_nSkin = FindInDataMap(player->GetPredDescMap(), str("m_nSkin"));
	int m_nBody = FindInDataMap(player->GetPredDescMap(), str("m_nBody"));

	hit.info.origin = player->GetAbsOrigin();
	hit.info.angles = player->GetAbsAngles();

	auto renderable = player->GetClientRenderable();

	if (!renderable)
		return;

	auto model = player->GetModel();

	if (!model)
		return;

	auto hdr = interfaces.models.model_info->GetStudioModel(model);
	if (!hdr)
		return;

	hit.state.m_pStudioHdr = hdr;
	hit.state.m_pStudioHWData = interfaces.model_cache->GetHardwareData(model->studio);

	hit.state.m_pRenderable = renderable;
	hit.state.m_drawFlags = 0;

	hit.info.pRenderable = renderable;
	hit.info.pModel = model;
	hit.info.pLightingOffset = nullptr;
	hit.info.pLightingOrigin = nullptr;
	hit.info.hitboxset = player->GetHitboxSet();
	hit.info.skin = (int)(uintptr_t(player) + m_nSkin);
	hit.info.body = (int)(uintptr_t(player) + m_nBody);
	hit.info.entity_index = player->GetIndex();
	hit.info.instance = getvfunc<ModelInstanceHandle_t(__thiscall*)(void*) >(renderable, 30)(renderable);
	hit.info.flags = 0x1;

	hit.info.pModelToWorld = &hit.model_to_world;
	hit.state.m_pModelToWorld = &hit.model_to_world;
	Math::AngleMatrix(hit.info.angles, hit.info.origin, hit.model_to_world);
}

void CChams::OnPostScreenEffects() {
	if (m_Hitmatrix.empty())
		return;

	auto IsNullMaterial = [&]() {
		if (Default == nullptr ||
			Flat == nullptr ||
			Glass == nullptr ||
			Metallic == nullptr ||
			GlowFade == nullptr ||
			GlowLine == nullptr ||
			Wireframe == nullptr)
			return true;
		return false;
	};

	if (IsNullMaterial())
		return;

	static auto DrawModelExecute = g_pModelRenderHook->GetOriginal< DrawModelExecuteFn >(g_HookIndices[fnva1(hs::Hooked_DrawModelExecute.s().c_str())]);
	if (!interfaces.models.model_render)
		return;

	auto ctx = interfaces.material_system->GetRenderContext();

	if (!ctx)
		return;

	bool should_draw = false;
	auto it = m_Hitmatrix.begin();
	while (it != m_Hitmatrix.end()) {

		if (!it->state.m_pModelToWorld || !it->state.m_pRenderable || !it->state.m_pStudioHdr || !it->state.m_pStudioHWData ||
			!it->info.pRenderable || !it->info.pModelToWorld || !it->info.pModel) {
			++it;
			should_draw = false;
			continue;
		}
		auto ent = interfaces.ent_list->GetClientEntity(it->info.entity_index);
		if (!ent) {
			it = m_Hitmatrix.erase(it);
			should_draw = false;
			continue;
		}

		auto alpha = 1.0f;
		auto delta = interfaces.global_vars->realtime - it->time;
		if (delta > 0.0f) {
			alpha -= delta;
			if (delta > 1.0f) {
				it = m_Hitmatrix.erase(it);
				should_draw = false;
				continue;
			}
		}
		should_draw = true;

		chams_t* shot_struct = new chams_t();
		shot_struct->enable = should_draw && vars.visuals.chams[enemy_ragebot_shot].enable;
		shot_struct->material = vars.visuals.chams[enemy_ragebot_shot].material;
		shot_struct->overlay = vars.visuals.chams[enemy_ragebot_shot].overlay;
		shot_struct->phong_amount = vars.visuals.chams[enemy_ragebot_shot].phong_amount;
		shot_struct->rim_amount = vars.visuals.chams[enemy_ragebot_shot].rim_amount;

		shot_struct->material_color
			= vars.visuals.chams[enemy_ragebot_shot].material_color.manage_alpha(vars.visuals.chams[enemy_ragebot_shot].material_color.get_alpha() * alpha);

		shot_struct->glass_color
			= vars.visuals.chams[enemy_ragebot_shot].glass_color.manage_alpha(vars.visuals.chams[enemy_ragebot_shot].glass_color.get_alpha() * alpha);

		shot_struct->wireframe_color
			= vars.visuals.chams[enemy_ragebot_shot].wireframe_color.manage_alpha(vars.visuals.chams[enemy_ragebot_shot].wireframe_color.get_alpha() * alpha);

		shot_struct->metallic_color
			= vars.visuals.chams[enemy_ragebot_shot].metallic_color.manage_alpha(vars.visuals.chams[enemy_ragebot_shot].metallic_color.get_alpha() * alpha);

		shot_struct->glow_color[0]
			= vars.visuals.chams[enemy_ragebot_shot].glow_color[0].manage_alpha(vars.visuals.chams[enemy_ragebot_shot].glow_color[0].get_alpha() * alpha);

		shot_struct->glow_color[1]
			= vars.visuals.chams[enemy_ragebot_shot].glow_color[1].manage_alpha(vars.visuals.chams[enemy_ragebot_shot].glow_color[1].get_alpha() * alpha);

		DrawChams(*shot_struct, it->pBoneToWorld, true,
			DrawModelExecute, interfaces.models.model_render, ctx, it->state, it->info);

		delete shot_struct;
		++it;
	}
}

// cache our materials
void CChams::Init()
{
	this->Default = CreateMaterial(true, str(R"#("VertexLitGeneric" {
			"$basetexture"					"vgui/white_additive"
			"$nofog"						"1"
			"$model"						"1"
			"$nocull"						"0"
			"$selfillum"					"1"
			"$halflambert"					"1"
			"$znearer"						"0"
			"$flat"							"1"
		})#"));

	this->Flat = CreateMaterial(false, str(R"#("UnlitGeneric" {
			"$basetexture"					"vgui/white_additive"
			"$nofog"						"1"
			"$model"						"1"
			"$nocull"						"0"
			"$selfillum"					"1"
			"$halflambert"					"1"
			"$znearer"						"0"
			"$flat"							"1"
		})#"));

	this->Wireframe = CreateMaterial(false, str(R"#("UnlitGeneric" {
			"$basetexture"					"vgui/white_additive"
			"$nofog"						"1"
			"$model"						"1"
			"$nocull"						"0"
			"$selfillum"					"1"
			"$halflambert"					"1"
			"$znearer"						"0"
			"$flat"							"1"
			"$wireframe"					"1"
		})#"));

	this->GlowFade = CreateMaterial(true, str(R"#("VertexLitGeneric" {
			"$additive"						"1"
			"$envmap"						"models/effects/cube_white"
			"$envmaptint"					"[1 1 1]"
			"$envmapfresnel"				"1"
			"$envmapfresnelminmaxexp"		"[0 1 2]"
			"$alpha" "1"
		})#"));

	this->GlowLine = interfaces.material_system->FindMaterial(hs::glow_armsrace.s().c_str(), nullptr);

	this->Glass = CreateMaterial(true, str(R"#("VertexLitGeneric" {
            "$basetexture"						"vgui/white_additive"
            "$envmap"							"env_cubemap"
            "$normalmapalphaenvmapmask"			"1"
            "$envmapcontrast"					"1"
            "$nofog"							"1"
            "$model"							"1"
            "$nocull"							"0"
            "$selfillum"						"1"
            "$halflambert"						"1"
            "$znearer"							"0"
            "$flat"								"1"
        })#"));

	this->Metallic = CreateMaterial(true, str(R"#("VertexLitGeneric" {
			"$basetexture"				    "vgui/white"
			"$phong"						"1"
			"$phongexponent"				"10"
			"$phongboost"					"1.0"
			"$rimlight"					    "1"
			"$rimlightexponent"		        "1"
			"$rimlightboost"		        "1"
			"$model"						"1"
			"$nocull"						"0"
			"$halflambert"				    "0"
			"$lightwarptexture"             "metalic"
		})#"));
}

bool str_starts_with(const char* str, const char* search_str)
{
	bool ret = false;

	for (intptr_t i = 0;; ++i)
	{
		if (search_str[i] == '\0')
		{
			ret = true;
			break;
		}

		if (str[i] != search_str[i])
			goto out;

		if (str[i] == '\0')
			break;
	}

out:
	return ret;
}

// if we return false, game should render original models like player, weapon, etc.
// if we return true, we manually draw model that we need 'cause we call original function

bool CChams::ShouldDraw(DrawModelExecuteFn original,
	void* thisptr,
	void* ctx,
	const DrawModelState_t& state,
	const ModelRenderInfo_t& info,
	matrix* pCustomBoneToWorld) {

	static std::string models_player = str("models/player");
	static std::string _arms = str("arms");
	static std::string v_ = str("models/weapons/v_");
	static std::string models_weapons = str("models/weapons/w_");

	auto Entity = interfaces.ent_list->GetClientEntity(info.entity_index);

	const auto ModelName = info.pModel->name;
	bool isModelRelativeToPlayer = str_starts_with(ModelName, models_player.c_str());
	if (isModelRelativeToPlayer)
	{
		if (Entity == nullptr
			|| csgo->local == nullptr)
			return false;

		if ([&]() {
			if (!Entity->IsPlayer())
				return false;
				if (!Entity->isAlive())
					return false;
				if (Entity->HasGunGameImmunity())
					return false;
				if (Entity->IsDormant())
					return false;
				if (Entity == csgo->local)
					return false;
				if (Entity->GetTeam() == csgo->local->GetTeam())
					return false;

			return true;
			}()) {

			/*auto latest_animation = g_Animfix->get_latest_animation(Entity);

			if (latest_animation) {
				auto pizda = Flat;
				pizda->AlphaModulate(0.5f);
				pizda->ColorModulate(1.f, 0.f, 0.f);

				interfaces.models.model_render->ForcedMaterialOverride(pizda);
				original(interfaces.models.model_render, ctx, state, info, latest_animation->unresolved_bones);


				pizda->ColorModulate(0.f, 0.f, 1.f);
				interfaces.models.model_render->ForcedMaterialOverride(pizda);
				original(interfaces.models.model_render, ctx, state, info, latest_animation->inversed_bones);


				interfaces.models.model_render->ForcedMaterialOverride(nullptr);
			}*/

			matrix HistoryMatrix[128] = {};
			if (vars.visuals.chams[enemy_history].enable && GetBacktrackMaxtrix(Entity, HistoryMatrix))
				DrawChams(vars.visuals.chams[enemy_history], HistoryMatrix, true, original, thisptr, ctx, state, info);

			bool ThroughWalls = DrawChams(vars.visuals.chams[enemy_xqz], pCustomBoneToWorld, true, original, thisptr, ctx, state, info);
			bool Visible = DrawChams(vars.visuals.chams[enemy_visible], pCustomBoneToWorld, false, original, thisptr, ctx, state, info);

			if (Visible || ThroughWalls)
				return true;
			return false;
		}
		else
		{
			if (Entity->EntIndex() == interfaces.engine->GetLocalPlayer()
				&& interfaces.input->m_fCameraInThirdPerson) {

				if (csgo->local && csgo->local->isAlive()) {

					if (vars.visuals.blend_on_scope && csgo->local->IsScoped())
						interfaces.render_view->SetBlend(vars.visuals.blend_value / 100.f);

					bool fr = csgo->game_rules->IsFreezeTime()
						|| csgo->local->HasGunGameImmunity()
						|| csgo->local->GetFlags() & FL_FROZEN;

					if (csgo->fakematrix)
					{
						for (auto& i : csgo->fakematrix)
						{
							i[0][3] += csgo->local->GetRenderOrigin().x;
							i[1][3] += csgo->local->GetRenderOrigin().y;
							i[2][3] += csgo->local->GetRenderOrigin().z;
						}

						if (!fr && abs(csgo->desync_angle) > 0.f)
							DrawChams(vars.visuals.chams[local_desync], csgo->fakematrix, false, original, thisptr, ctx, state, info);

						for (auto& i : csgo->fakematrix)
						{
							i[0][3] -= csgo->local->GetRenderOrigin().x;
							i[1][3] -= csgo->local->GetRenderOrigin().y;
							i[2][3] -= csgo->local->GetRenderOrigin().z;
						}
					}

					bool LocalVisible = DrawChams(vars.visuals.chams[local_default], pCustomBoneToWorld, false, original, thisptr, ctx, state, info);
					if (LocalVisible)
						return true;

					return false;
				}
			}
		}

		return false;
	}
	else if (strstr(ModelName, _arms.c_str()) != nullptr) {
		bool Visible = DrawChams(vars.visuals.chams[local_arms], pCustomBoneToWorld, false, original, thisptr, ctx, state, info);
		return Visible;
	}
	else if (strstr(ModelName, v_.c_str()) != nullptr) {
		bool Visible = DrawChams(vars.visuals.chams[local_weapon], pCustomBoneToWorld, false, original, thisptr, ctx, state, info);
		return Visible;
	}

	return false;
}