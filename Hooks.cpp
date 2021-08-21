 #include "Hooks.h"
#include "netvar_manager.h"

// Include hooks
#include "PaintTraverse.h"
#include "Createmove.h"
#include "FrameStageNotify.h"
#include "EndScene.h"
#include "WndProc.h"
#include "SceneEnd.h"
#include "DoPostScreenEffects.h"
#include "FireEventClientSide.h"
#include "BeginFrame.h"
#include "OverRideView.h"
#include "DrawModelExecuted.h"
#include "SetupAnimation.h"
#include "EngineHook.h"
#include "TraceRay.h"

#include "hooker.h"
#include <thread>

std::unique_ptr< VMTHook > g_pDirectXHook = nullptr;
std::unique_ptr< VMTHook > g_pClientStateAdd = nullptr;
std::unique_ptr< VMTHook > g_pPanelHook = nullptr;
std::unique_ptr< VMTHook > g_pClientModeHook = nullptr;
std::unique_ptr< VMTHook > g_pClientHook = nullptr;
std::unique_ptr< VMTHook > g_pRenderViewHook = nullptr;
std::unique_ptr< VMTHook > g_pStudioRenderHook = nullptr;
std::unique_ptr< VMTHook > g_pModelRenderHook = nullptr;
std::unique_ptr< VMTHook > g_pPredictHook = nullptr;
std::unique_ptr< VMTHook > g_pMovementHook = nullptr;
std::unique_ptr< VMTHook > g_pQueryHook = nullptr;
std::unique_ptr< VMTHook > g_pGetBoolHook = nullptr;
std::unique_ptr< VMTHook > g_pCameraHook = nullptr;
std::unique_ptr< VMTHook > g_pEngineHook = nullptr;
std::unique_ptr< VMTHook > g_pEngineTraceHook = nullptr;
std::unique_ptr< VMTHook > g_pLeafSystemHook = nullptr;
std::unique_ptr< VMTHook > g_pTEFireBullets = nullptr;
std::unique_ptr< VMTHook > g_pNetShowFragments = nullptr;
std::unique_ptr< VMTHook > g_pShadow = nullptr;
std::unique_ptr< VMTHook > g_pMaterialSystemHook = nullptr;
std::unique_ptr< VMTHook > g_pDirectHook = nullptr;

//ModifyEyePosFn H::ModifyEyePos;
//AddViewModelBobFn H::AddViewModelBob;
//CalcViewModelViewFn H::CalcViewModelView;
//ClMoveFn H::ClMove;
//SetupBonesFn H::SetupBones;
//DoExtraBoneProcessingFn H::DoExtraBoneProcessing;
//StandardBlendingRulesFn H::StandardBlendingRules;
//ShouldSkipAnimFrameFn H::ShouldSkipAnimFrame;
//UpdateClientSideAnimationFn H::UpdateClientSideAnimation;
//PhysicsSimulateFn H::PhysicsSimulate;
//PresentFn H::SteamPresent;
//ResetFn H::SteamReset;
//GetColorModulationFn H::GetColorModulation;
//IsUsingStaticPropDebugModesFn H::IsUsingStaticPropDebugModes;

CCheatVars* csgo = new CCheatVars();
CInterfaces interfaces;
std::vector<IGameEventListener2*> g_pGameEventManager;

void CCheatVars::updatelocalplayer()
{
	local = interfaces.ent_list->GetClientEntity(interfaces.engine->GetLocalPlayer());
}

void Msg(string str, color_t clr)
{
	g_Eventlog->messages.push_back(CMessage(str, csgo->get_absolute_time(), clr));
	interfaces.cvars->ConsoleColorPrintf(clr, hs::prefix.s().c_str());
	interfaces.cvars->ConsoleColorPrintf(color_t(255, 255, 255), hs::_s_n.s().c_str(), str.c_str());
}

template<typename T>
static T* GetInterface(const char* _module, const char* name)
{
	static unsigned int CreateInterfaceFn = reinterpret_cast<unsigned int>(GetProcAddress(GetModuleHandleA(_module), hs::create_interface.s().c_str()));
	if (!CreateInterfaceFn)
		return nullptr;

	T* inter = nullptr;
	int len = strlen(name);

	unsigned int target = (unsigned int)(CreateInterfaceFn)+*(unsigned int*)((unsigned int)(CreateInterfaceFn)+5) + 9;

	InterfaceReg* interfaceRegList = **reinterpret_cast<InterfaceReg***>(target + 6);

	for (InterfaceReg* cur = interfaceRegList; cur; cur = cur->m_pNext)
	{
		if (!strncmp(cur->m_pName, name, len) && std::atoi(cur->m_pName + len) > 0)
			inter = reinterpret_cast<T*>(cur->m_CreateFn());
	}
	return inter;
}

inline uintptr_t GetAbsoluteAddress(uintptr_t instruction_ptr, int offset, int size)
{
	return instruction_ptr + *reinterpret_cast<uint32_t*>(instruction_ptr + offset) + size;
};

template< typename T >
T* Interface(const char* strModule, const char* strInterface)
{
	typedef T* (*CreateInterfaceFn)(const char* szName, int iReturn);
	CreateInterfaceFn CreateInterface = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA(strModule), (hs::create_interface.s().c_str()));

	if (!CreateInterface)
		return nullptr;

	return CreateInterface(strInterface, 0);
}

void I::Setup()
{
	csgo->Utils.FindPatternIDA = [](HMODULE hModule, const char* szSignature) -> uint8_t* {
		static auto pattern_to_byte = [](const char* pattern) {
			auto bytes = std::vector<int>{};
			auto start = const_cast<char*>(pattern);
			auto end = const_cast<char*>(pattern) + strlen(pattern);

			for (auto current = start; current < end; ++current) {
				if (*current == '?') {
					++current;
					if (*current == '?')
						++current;
					bytes.push_back(-1);
				}
				else {
					bytes.push_back(strtoul(current, &current, 16));
				}
			}
			return bytes;
		};

		//auto Module = GetModuleHandleA(szModule);

		auto dosHeader = (PIMAGE_DOS_HEADER)hModule;
		auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)hModule + dosHeader->e_lfanew);

		auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
		auto patternBytes = pattern_to_byte(szSignature);
		auto scanBytes = reinterpret_cast<std::uint8_t*>(hModule);

		auto s = patternBytes.size();
		auto d = patternBytes.data();

		for (auto i = 0ul; i < sizeOfImage - s; ++i) {
			bool found = true;
			for (auto j = 0ul; j < s; ++j) {
				if (scanBytes[i + j] != d[j] && d[j] != -1) {
					found = false;
					break;
				}
			}
			if (found) {
				return &scanBytes[i];
			}
		}
		return nullptr;
	};

	[]() {
		g_Modules[fnva1(hs::serverbrowser_dll.s().c_str())] = hs::serverbrowser_dll.s().c_str();
		g_Modules[fnva1(hs::client_dll.s().c_str())] = hs::client_dll.s().c_str();
		g_Modules[fnva1(hs::engine_dll.s().c_str())] = hs::engine_dll.s().c_str();
		g_Modules[fnva1(hs::vstdlib_dll.s().c_str())] = hs::vstdlib_dll.s().c_str();
		g_Modules[fnva1(hs::input_system_dll.s().c_str())] = hs::input_system_dll.s().c_str();
		g_Modules[fnva1(hs::server_dll.s().c_str())] = hs::server_dll.s().c_str();
		g_Modules[fnva1(hs::vgui_mat_surface_dll.s().c_str())] = hs::vgui_mat_surface_dll.s().c_str();
		g_Modules[fnva1(hs::vgui2_dll.s().c_str())] = hs::vgui2_dll.s().c_str();
		g_Modules[fnva1(hs::mat_sys_dll.s().c_str())] = hs::mat_sys_dll.s().c_str();
		g_Modules[fnva1(hs::studio_render_dll.s().c_str())] = hs::studio_render_dll.s().c_str();
		g_Modules[fnva1(hs::physics_dll.s().c_str())] = hs::physics_dll.s().c_str();
		g_Modules[fnva1(hs::data_cache_dll.s().c_str())] = hs::data_cache_dll.s().c_str();
		g_Modules[fnva1(hs::tier0_dll.s().c_str())] = hs::tier0_dll.s().c_str();
		g_Modules[fnva1(hs::gameoverlayrenderer_dll.s().c_str())] = hs::gameoverlayrenderer_dll.s().c_str();
		g_Modules[fnva1(hs::shaderapidx9_dll.s().c_str())] = hs::shaderapidx9_dll.s().c_str();
	}();


	g_Patterns[fnva1(hs::client_state.s().c_str())] = hs::client_state_p.s().c_str();
	g_Patterns[fnva1(hs::move_helper.s().c_str())] = hs::move_helper_p.s().c_str();
	g_Patterns[fnva1(hs::reset.s().c_str())] = hs::reset_p.s().c_str();
	g_Patterns[fnva1(hs::input.s().c_str())] = hs::input_p.s().c_str();
	g_Patterns[fnva1(hs::glow_manager.s().c_str())] = hs::glow_manager_p.s().c_str();
	g_Patterns[fnva1(hs::beams.s().c_str())] = hs::beams_p.s().c_str();
	g_Patterns[fnva1(hs::update_clientside_anim.s().c_str())] = hs::update_clientside_anim_p.s().c_str();
	g_Patterns[fnva1(hs::debp.s().c_str())] = hs::debp_p.s().c_str();
	g_Patterns[fnva1(hs::standard_blending_rules.s().c_str())] = hs::standard_blending_rules_p.s().c_str();
	g_Patterns[fnva1(hs::should_skip_anim_frame.s().c_str())] = hs::should_skip_anim_frame_p.s().c_str();
	g_Patterns[fnva1(hs::get_foreign_fall_back_name.s().c_str())] = hs::get_foreign_fall_back_name_p.s().c_str();
	g_Patterns[fnva1(hs::setup_bones.s().c_str())] = hs::setup_bones_p.s().c_str();
	g_Patterns[fnva1(hs::build_transformations.s().c_str())] = hs::build_transformations_p.s().c_str();
	g_Patterns[fnva1(hs::effects.s().c_str())] = hs::effects_p.s().c_str();

	[]() {
		g_HookIndices[fnva1(hs::Hooked_ProcessMovement.s().c_str())] = 1;
		g_HookIndices[fnva1(hs::Hooked_SendDatagram.s().c_str())] = 46;
		g_HookIndices[fnva1(hs::Hooked_GetColorModulation.s().c_str())] = 47;
		g_HookIndices[fnva1(hs::Hooked_FrameStageNotify.s().c_str())] = 37;
		g_HookIndices[fnva1(hs::Hooked_WriteUsercmdDeltaToBuffer.s().c_str())] = 24;
		g_HookIndices[fnva1(hs::Hooked_IsPaused.s().c_str())] = 90;
		g_HookIndices[fnva1(hs::Hooked_IsBoxVisible.s().c_str())] = 32;
		g_HookIndices[fnva1(hs::Hooked_IsHLTV.s().c_str())] = 93;
		g_HookIndices[fnva1(hs::Hooked_GetScreenAspectRatio.s().c_str())] = 101;
		g_HookIndices[fnva1(hs::Hooked_ShouldDrawShadow.s().c_str())] = 13;
		g_HookIndices[fnva1(hs::Hooked_CreateMove.s().c_str())] = 24;
		g_HookIndices[fnva1(hs::Hooked_ShouldDrawFog.s().c_str())] = 17;
		g_HookIndices[fnva1(hs::Hooked_OverrideView.s().c_str())] = 18;
		g_HookIndices[fnva1(hs::Hooked_ViewModel.s().c_str())] = 35;
		g_HookIndices[fnva1(hs::Hooked_DoPostScreenEffects.s().c_str())] = 44;
		g_HookIndices[fnva1(hs::Hooked_PaintTraverse.s().c_str())] = 41;
		g_HookIndices[fnva1(hs::Hooked_BeginFrame.s().c_str())] = 9;
		g_HookIndices[fnva1(hs::Hooked_PacketStart.s().c_str())] = 5;
		g_HookIndices[fnva1(hs::Hooked_PacketEnd.s().c_str())] = 6;
		g_HookIndices[fnva1(hs::Hooked_DrawModelExecute.s().c_str())] = 21;
		g_HookIndices[fnva1(hs::Hooked_RunCommand.s().c_str())] = 19;
		g_HookIndices[fnva1(hs::Hooked_InPrediction.s().c_str())] = 14;
		g_HookIndices[fnva1(hs::Hooked_ListLeavesInBox.s().c_str())] = 6;
		g_HookIndices[fnva1(hs::Hooked_GetBool.s().c_str())] = 13;
		g_HookIndices[fnva1(hs::Hooked_SceneEnd.s().c_str())] = 9;
		g_HookIndices[fnva1(hs::Hooked_ClipRayColliedable.s().c_str())] = 4;
		g_HookIndices[fnva1(hs::Hooked_EndScene.s().c_str())] = 42;
		g_HookIndices[fnva1(hs::Hooked_Reset.s().c_str())] = 16;
	}();

	[]() {
		interfaces.client = Interface< IBaseClientDll >((
			g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
			hs::client.s().c_str());

		interfaces.ent_list = Interface< CEntityList >((
			g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
			hs::ent_list.s().c_str());

		interfaces.leaf_system = Interface< IClientLeafSystem >((
			g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
			hs::leaf_sys.s().c_str());

		interfaces.cvars = Interface< ICVar >(
			g_Modules[fnva1(hs::vstdlib_dll.s().c_str())]().c_str(),
			hs::cvar.s().c_str());

		interfaces.engine = Interface< IEngineClient >(((
			g_Modules[fnva1(hs::engine_dll.s().c_str())]().c_str())),
			hs::engine_client.s().c_str());

		interfaces.trace = Interface< IEngineTrace >(((
			g_Modules[fnva1(hs::engine_dll.s().c_str())]().c_str())),
			hs::engine_trace.s().c_str());

		interfaces.inputsystem = Interface< IInputSystem >(
			g_Modules[fnva1(hs::input_system_dll.s().c_str())]().c_str(),
			hs::input_sys.s().c_str());

		interfaces.surfaces = Interface< ISurface >(
			g_Modules[fnva1(hs::vgui_mat_surface_dll.s().c_str())]().c_str(),
			hs::vgui_surface.s().c_str());

		interfaces.server = GetInterface<IServerGameDLL>(
			g_Modules[fnva1(hs::server_dll.s().c_str())]().c_str(),
			hs::server_game_dll.s().c_str());

		interfaces.v_panel = Interface< IVPanel >(
			g_Modules[fnva1(hs::vgui2_dll.s().c_str())]().c_str(),
			hs::vgui_panel.s().c_str());

		interfaces.models.model_render = Interface< IVModelRender >(((
			g_Modules[fnva1(hs::engine_dll.s().c_str())]().c_str())),
			hs::engine_model.s().c_str());

		interfaces.material_system = Interface< IMaterialSystem >(
			g_Modules[fnva1(hs::mat_sys_dll.s().c_str())]().c_str(),
			hs::mat_sys.s().c_str());

		interfaces.models.model_info = Interface< IVModelInfo >(((
			g_Modules[fnva1(hs::engine_dll.s().c_str())]().c_str())),
			hs::model_info.s().c_str());

		interfaces.render_view = Interface< IVRenderView >(((
			g_Modules[fnva1(hs::engine_dll.s().c_str())]().c_str())),
			hs::render_view.s().c_str());

		interfaces.debug_overlay = Interface< IVDebugOverlay >(((
			g_Modules[fnva1(hs::engine_dll.s().c_str())]().c_str())),
			hs::debug_overlay.s().c_str());

		interfaces.event_manager = Interface< IGameEventManager2 >(((
			g_Modules[fnva1(hs::engine_dll.s().c_str())]().c_str())),
			hs::events_manager.s().c_str());

		interfaces.studio_render = Interface< IStudioRender >(
			g_Modules[fnva1(hs::studio_render_dll.s().c_str())]().c_str(),
			hs::studio_render.s().c_str());

		interfaces.phys_props = Interface< IPhysicsSurfaceProps >(
			g_Modules[fnva1(hs::physics_dll.s().c_str())]().c_str(),
			hs::phys_props.s().c_str());

		interfaces.game_movement = Interface< CGameMovement >((
			g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
			hs::game_movement.s().c_str());

		interfaces.model_cache = Interface< IMDLCache >(
			g_Modules[fnva1(hs::data_cache_dll.s().c_str())]().c_str(),
			hs::mdl_cache.s().c_str());

		interfaces.engine_vgui = Interface<IEngineVGui>((
			g_Modules[fnva1(hs::engine_dll.s().c_str())]().c_str()),
			hs::engine_vgui.s().c_str());

		interfaces.engine_sound = Interface< IEngineSound >((
			g_Modules[fnva1(hs::engine_dll.s().c_str())]().c_str()),
			hs::engine_sound.s().c_str());

		interfaces.net_string_container = Interface< CNetworkStringTableContainer >((
			g_Modules[fnva1(hs::engine_dll.s().c_str())]().c_str()),
			hs::string_table.s().c_str());

		interfaces.move_helper = **reinterpret_cast<IMoveHelper***>(csgo->Utils.FindPatternIDA(GetModuleHandleA(
			g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
			g_Patterns[fnva1(hs::move_helper.s().c_str())]().c_str()) + 2);

		interfaces.prediction = Interface< CPrediction >((
			g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
			hs::client_prediction.s().c_str());

		interfaces.input = reinterpret_cast<CInput*>(*reinterpret_cast<DWORD*>(csgo->Utils.FindPatternIDA(GetModuleHandleA(
			g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
			g_Patterns[fnva1(hs::input.s().c_str())]().c_str()) + 1));

		interfaces.glow_manager = *reinterpret_cast<CGlowObjectManager**>(csgo->Utils.FindPatternIDA(GetModuleHandleA(
			g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
			g_Patterns[fnva1(hs::glow_manager.s().c_str())]().c_str()) + 3);

		interfaces.beams = *reinterpret_cast<IViewRenderBeams**>(csgo->Utils.FindPatternIDA(GetModuleHandleA(
			g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
			g_Patterns[fnva1(hs::beams.s().c_str())]().c_str()) + 1);

		interfaces.memalloc = *reinterpret_cast<IMemAlloc**>(GetProcAddress(GetModuleHandleA(
			g_Modules[fnva1(hs::tier0_dll.s().c_str())]().c_str()),
			hs::mem_alloc.s().c_str()));
	}();
	interfaces.global_vars = **reinterpret_cast <CGlobalVars***> ((*(DWORD**)interfaces.client)[0] + 0x1F);
	interfaces.client_mode = **reinterpret_cast<IClientMode***>((*reinterpret_cast<uintptr_t**>(interfaces.client))[10] + 0x5u);
}

bool F::Shooting()
{
	if (!csgo->weapon)
		return false;
	if (csgo->weapon->IsBomb())
		return false;
	else if (csgo->weapon->IsNade())
	{
		CBaseCSGrenade* csGrenade = (CBaseCSGrenade*)csgo->weapon;
		if (csGrenade->GetThrowTime() > 0.f)
		{
			return true;
		}
	}
	else if (csgo->weapon->IsKnife()) {
		if (csgo->cmd->buttons & IN_ATTACK || csgo->cmd->buttons & IN_ATTACK2 && g_Ragebot->IsAbleToShoot())
			return true;
	}
	else {
		if (csgo->cmd->buttons & IN_ATTACK && g_Ragebot->IsAbleToShoot())
			return true;
		return false;
	}
	return false;
}

void H::Hook()
{
	g_EnginePrediction->m_hConstraintEntity = netvars.GetOffset(str("DT_BasePlayer"), str("m_hConstraintEntity"));
	g_EnginePrediction->initalize();

	memset(csgo->key_pressed, 0, sizeof(csgo->key_pressed));
	memset(csgo->key_down, 0, sizeof(csgo->key_down));
	memset(csgo->key_pressedticks, 0, sizeof(csgo->key_pressedticks));

	g_ClassID = new ClassIdManager();
	
	csgo->client_state = **reinterpret_cast<CClientState***>(
		csgo->Utils.FindPatternIDA(
			GetModuleHandleA(g_Modules[fnva1(hs::engine_dll.s().c_str())]().c_str()),
			g_Patterns[fnva1(hs::client_state.s().c_str())]().c_str()) + 1);

	DWORD dwDirectDevice = **reinterpret_cast<DWORD**>(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::shaderapidx9_dll.s().c_str())]().c_str()),
		hs::d3d_device.s().c_str()) + 1);

	DWORD* dwUpdateClientSideAnimation = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		g_Patterns[fnva1(hs::update_clientside_anim.s().c_str())]().c_str()));

	DWORD* dwDoExtraBoneProcessing = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		g_Patterns[fnva1(hs::debp.s().c_str())]().c_str()));

	DWORD* dwStandardBlendingRules = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		g_Patterns[fnva1(hs::standard_blending_rules.s().c_str())]().c_str()));

	DWORD* dwShouldSkipAnimFrame = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		g_Patterns[fnva1(hs::should_skip_anim_frame.s().c_str())]().c_str()));

	DWORD* dwPhysicsSimulate = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::physicssimulate.s().c_str()));

	DWORD* dwSetupBones = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		g_Patterns[fnva1(hs::setup_bones.s().c_str())]().c_str()));

	DWORD* dwBuildTransform = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		g_Patterns[fnva1(hs::build_transformations.s().c_str())]().c_str()));

	DWORD* dwAddViewModelBob = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::add_view_model_bob.s().c_str()));

	DWORD* dwEyePos = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::eye_pos.s().c_str()));

	DWORD* dwCLMove = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::engine_dll.s().c_str())]().c_str()),
		hs::cl_move.s().c_str()));

	DWORD* dwCalcViewModelView = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::calc_view_model_view.s().c_str()));

	DWORD* dwColorModulation = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::mat_sys_dll.s().c_str())]().c_str()),
		hs::get_clr_modulation.s().c_str()));

	DWORD* dwIsUsingStaticPropDebugModes = (DWORD*)(csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::engine_dll.s().c_str())]().c_str()),
		hs::using_static_prop.s().c_str()));

	auto sv_cheats = interfaces.cvars->FindVar(hs::sv_cheats.s().c_str());
	auto camera = interfaces.cvars->FindVar(hs::camera.s().c_str());
	auto shadows = interfaces.cvars->FindVar(hs::shadows.s().c_str());

	g_pClientStateAdd = std::make_unique< VMTHook >();
	g_pEngineHook = std::make_unique< VMTHook >();
	g_pPanelHook = std::make_unique< VMTHook >();
	g_pClientModeHook = std::make_unique< VMTHook >();
	g_pShadow = std::make_unique< VMTHook >();
	g_pStudioRenderHook = std::make_unique< VMTHook >();
	g_pModelRenderHook = std::make_unique< VMTHook >();
	g_pPredictHook = std::make_unique< VMTHook >();
	g_pMovementHook = std::make_unique< VMTHook >();
	g_pQueryHook = std::make_unique< VMTHook >();
	g_pCameraHook = std::make_unique< VMTHook >();
	g_pGetBoolHook = std::make_unique< VMTHook >();
	g_pDirectXHook = std::make_unique< VMTHook >();
	g_pRenderViewHook = std::make_unique< VMTHook >();
	g_pLeafSystemHook = std::make_unique< VMTHook >();
	g_pClientHook = std::make_unique< VMTHook >();
	g_pMaterialSystemHook = std::make_unique< VMTHook >();
	g_pEngineTraceHook = std::make_unique< VMTHook >();
	g_pDirectHook = std::make_unique< VMTHook >();

	g_pEngineHook->Setup(interfaces.engine);
	g_pPanelHook->Setup(interfaces.v_panel);
	g_pClientHook->Setup(interfaces.client);
	g_pClientModeHook->Setup(interfaces.client_mode);
	g_pShadow->Setup(shadows);
	g_pStudioRenderHook->Setup(interfaces.studio_render);
	g_pModelRenderHook->Setup(interfaces.models.model_render);
	g_pPredictHook->Setup(interfaces.prediction);
	g_pMovementHook->Setup(interfaces.game_movement);
	g_pQueryHook->Setup(interfaces.engine->GetBSPTreeQuery());
	g_pCameraHook->Setup(camera);
	g_pGetBoolHook->Setup(sv_cheats);
	g_pRenderViewHook->Setup(interfaces.render_view);
	g_pLeafSystemHook->Setup(interfaces.leaf_system);
	g_pMaterialSystemHook->Setup(interfaces.material_system);
	g_pClientStateAdd->Setup((CClientState*)(uint32_t(csgo->client_state) + 0x8));
	g_pEngineTraceHook->Setup(interfaces.trace);
	g_pDirectHook->Setup((void*)dwDirectDevice);

	g_pPanelHook->Hook(g_HookIndices[fnva1(hs::Hooked_PaintTraverse.s().c_str())], Hooked_PaintTraverse);
	g_pClientHook->Hook(g_HookIndices[fnva1(hs::Hooked_FrameStageNotify.s().c_str())], Hooked_FrameStageNotify);
	g_pClientHook->Hook(g_HookIndices[fnva1(hs::Hooked_WriteUsercmdDeltaToBuffer.s().c_str())], Hooked_WriteUsercmdDeltaToBuffer);
	g_pEngineHook->Hook(g_HookIndices[fnva1(hs::Hooked_IsPaused.s().c_str())], Hooked_IsPaused);
	g_pEngineHook->Hook(g_HookIndices[fnva1(hs::Hooked_IsBoxVisible.s().c_str())], Hooked_IsBoxVisible);
	g_pEngineHook->Hook(g_HookIndices[fnva1(hs::Hooked_IsHLTV.s().c_str())], Hooked_IsHLTV);
	g_pEngineHook->Hook(g_HookIndices[fnva1(hs::Hooked_GetScreenAspectRatio.s().c_str())], Hooked_GetScreenAspectRatio);
	g_pShadow->Hook(g_HookIndices[fnva1(hs::Hooked_ShouldDrawShadow.s().c_str())], Hooked_ShouldDrawShadow);
	g_pClientModeHook->Hook(g_HookIndices[fnva1(hs::Hooked_CreateMove.s().c_str())], Hooked_CreateMove);
	g_pClientModeHook->Hook(g_HookIndices[fnva1(hs::Hooked_ShouldDrawFog.s().c_str())], Hooked_ShouldDrawFog);
	g_pClientModeHook->Hook(g_HookIndices[fnva1(hs::Hooked_OverrideView.s().c_str())], Hooked_OverrideView);
	g_pClientModeHook->Hook(g_HookIndices[fnva1(hs::Hooked_ViewModel.s().c_str())], Hooked_ViewModel);
	g_pClientModeHook->Hook(g_HookIndices[fnva1(hs::Hooked_DoPostScreenEffects.s().c_str())], Hooked_DoPostScreenEffects);
	g_pStudioRenderHook->Hook(g_HookIndices[fnva1(hs::Hooked_BeginFrame.s().c_str())], Hooked_BeginFrame);
	g_pClientStateAdd->Hook(g_HookIndices[fnva1(hs::Hooked_PacketStart.s().c_str())], Hooked_PacketStart);
	g_pClientStateAdd->Hook(g_HookIndices[fnva1(hs::Hooked_PacketEnd.s().c_str())], Hooked_PacketEnd);
	g_pModelRenderHook->Hook(g_HookIndices[fnva1(hs::Hooked_DrawModelExecute.s().c_str())], Hooked_DrawModelExecute);
	g_pMovementHook->Hook(g_HookIndices[fnva1(hs::Hooked_ProcessMovement.s().c_str())], Hooked_ProcessMovement);
	g_pPredictHook->Hook(g_HookIndices[fnva1(hs::Hooked_RunCommand.s().c_str())], Hooked_RunCommand);
	g_pPredictHook->Hook(g_HookIndices[fnva1(hs::Hooked_InPrediction.s().c_str())], Hooked_InPrediction);
	g_pQueryHook->Hook(g_HookIndices[fnva1(hs::Hooked_ListLeavesInBox.s().c_str())], Hooked_ListLeavesInBox);
	g_pGetBoolHook->Hook(g_HookIndices[fnva1(hs::Hooked_GetBool.s().c_str())], Hooked_GetBool);
	g_pRenderViewHook->Hook(g_HookIndices[fnva1(hs::Hooked_SceneEnd.s().c_str())], Hooked_SceneEnd);

	g_pDirectHook->Hook(g_HookIndices[fnva1(hs::Hooked_EndScene.s().c_str())], Hooked_EndScene);
	g_pDirectHook->Hook(g_HookIndices[fnva1(hs::Hooked_Reset.s().c_str())], Hooked_Reset);

	hooker::h.create_hook(Hooked_UpdateClientSideAnimation, dwUpdateClientSideAnimation);
	hooker::h.create_hook(Hooked_DoExtraBoneProcessing, dwDoExtraBoneProcessing);
	hooker::h.create_hook(Hooked_ShouldSkipAnimFrame, dwShouldSkipAnimFrame);
	hooker::h.create_hook(Hooked_StandardBlendingRules, dwStandardBlendingRules);
	hooker::h.create_hook(Hooked_CalcViewModelView, dwCalcViewModelView);
	hooker::h.create_hook(Hooked_AddViewModelBob, dwAddViewModelBob);
	hooker::h.create_hook(Hooked_SetupBones, dwSetupBones);
	hooker::h.create_hook(Hooked_GetColorModulation, dwColorModulation);
	hooker::h.create_hook(Hooked_IsUsingStaticPropDebugModes, dwIsUsingStaticPropDebugModes);
//	hooker::h.create_hook(Hooked_PhysicsSimulate, dwPhysicsSimulate);
	hooker::h.create_hook(Hooked_ModifyEyePosition, dwEyePos);
//	hooker::h.create_hook(Hooked_CLMove, dwCLMove);

	hooker::h.enable();

	if (csgo->Init.Window)
		csgo->Init.OldWindow = (WNDPROC)SetWindowLongPtr(csgo->Init.Window, GWL_WNDPROC, (LONG_PTR)Hooked_WndProc);
	
	auto address = csgo->Utils.FindPatternIDA(
		GetModuleHandleA(g_Modules[fnva1(hs::engine_dll.s().c_str())]().c_str()),
		hs::choke_limit.s().c_str()) + 0x1;

	uint32_t choke_clamp = 17;

	DWORD oldProtect = 0;
	VirtualProtect((void*)address, sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &oldProtect);
	*(uint32_t*)address = choke_clamp;
	VirtualProtect((void*)address, sizeof(uint32_t), oldProtect, &oldProtect);

	RegListeners();
}

void H::UnHook()
{
	if (interfaces.inputsystem)
		interfaces.inputsystem->EnableInput(true);

	if (g_Animfix->FakeAnimstate)
		interfaces.memalloc->Free(g_Animfix->FakeAnimstate);

	hooker::h.restore();

	g_pDirectHook->Unhook(g_HookIndices[fnva1(hs::Hooked_Reset.s().c_str())]);
	g_pDirectHook->Unhook(g_HookIndices[fnva1(hs::Hooked_EndScene.s().c_str())]);
	g_pClientStateAdd->Unhook(g_HookIndices[fnva1(hs::Hooked_PacketStart.s().c_str())]);
	g_pClientStateAdd->Unhook(g_HookIndices[fnva1(hs::Hooked_PacketEnd.s().c_str())]);
	g_pClientHook->Unhook(g_HookIndices[fnva1(hs::Hooked_FrameStageNotify.s().c_str())]);
	g_pClientHook->Unhook(g_HookIndices[fnva1(hs::Hooked_WriteUsercmdDeltaToBuffer.s().c_str())]);
	g_pPanelHook->Unhook(g_HookIndices[fnva1(hs::Hooked_PaintTraverse.s().c_str())]);
	g_pEngineHook->Unhook(g_HookIndices[fnva1(hs::Hooked_IsPaused.s().c_str())]);
	g_pEngineHook->Unhook(g_HookIndices[fnva1(hs::Hooked_IsBoxVisible.s().c_str())]);
	g_pEngineHook->Unhook(g_HookIndices[fnva1(hs::Hooked_IsHLTV.s().c_str())]);
	g_pEngineHook->Unhook(g_HookIndices[fnva1(hs::Hooked_GetScreenAspectRatio.s().c_str())]);
	g_pShadow->Unhook(g_HookIndices[fnva1(hs::Hooked_ShouldDrawShadow.s().c_str())]);
	g_pClientModeHook->Unhook(g_HookIndices[fnva1(hs::Hooked_ShouldDrawFog.s().c_str())]);
	g_pClientModeHook->Unhook(g_HookIndices[fnva1(hs::Hooked_OverrideView.s().c_str())]);
	g_pClientModeHook->Unhook(g_HookIndices[fnva1(hs::Hooked_CreateMove.s().c_str())]);
	g_pClientModeHook->Unhook(g_HookIndices[fnva1(hs::Hooked_OverrideView.s().c_str())]);
	g_pClientModeHook->Unhook(g_HookIndices[fnva1(hs::Hooked_DoPostScreenEffects.s().c_str())]);
	g_pStudioRenderHook->Unhook(g_HookIndices[fnva1(hs::Hooked_BeginFrame.s().c_str())]);
	g_pModelRenderHook->Unhook(g_HookIndices[fnva1(hs::Hooked_DrawModelExecute.s().c_str())]);
	g_pPredictHook->Unhook(g_HookIndices[fnva1(hs::Hooked_RunCommand.s().c_str())]);
	g_pPredictHook->Unhook(g_HookIndices[fnva1(hs::Hooked_InPrediction.s().c_str())]);
	g_pRenderViewHook->Unhook(g_HookIndices[fnva1(hs::Hooked_SceneEnd.s().c_str())]);
	g_pQueryHook->Unhook(g_HookIndices[fnva1(hs::Hooked_ListLeavesInBox.s().c_str())]);
	g_pGetBoolHook->Unhook(g_HookIndices[fnva1(hs::Hooked_GetBool.s().c_str())]);
	g_pMovementHook->Unhook(g_HookIndices[fnva1(hs::Hooked_ProcessMovement.s().c_str())]);

	for (auto listener : g_pGameEventManager)
		interfaces.event_manager->RemoveListener(listener);
}