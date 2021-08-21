#include "Math.h"
#include "render.h"
#include "interfaces.h"
#include "VMT_Manager.h"
#include "hooker.h"
#include "Variables.h"
#include "Chams.h"
#include "Glow.h"
#include "javascript/js_handler.h"


#pragma comment(lib, "ntdll.lib")

extern "C" NTSYSAPI NTSTATUS NTAPI NtWriteVirtualMemory(
	IN HANDLE ProcessHandle,
	IN PVOID BaseAddress,
	IN PVOID Buffer,
	IN ULONG NumberOfBytesToWrite,
	OUT PULONG NumberOfBytesWritten OPTIONAL
);

extern std::unique_ptr< VMTHook > g_pDirectXHook;
extern std::unique_ptr< VMTHook > g_pClientStateAdd;
extern std::unique_ptr< VMTHook > g_pPanelHook;
extern std::unique_ptr< VMTHook > g_pClientModeHook;
extern std::unique_ptr< VMTHook > g_pClientHook;
extern std::unique_ptr< VMTHook > g_pRenderViewHook;
extern std::unique_ptr< VMTHook > g_pStudioRenderHook;
extern std::unique_ptr< VMTHook > g_pModelRenderHook;
extern std::unique_ptr< VMTHook > g_pMaterialSystemHook;
extern std::unique_ptr< VMTHook > g_pPredictHook;
extern std::unique_ptr< VMTHook > g_pMovementHook;
extern std::unique_ptr< VMTHook > g_pQueryHook;
extern std::unique_ptr< VMTHook > g_pGetBoolHook;
extern std::unique_ptr< VMTHook > g_pCameraHook;
extern std::unique_ptr< VMTHook > g_pEngineHook;
extern std::unique_ptr< VMTHook > g_pEngineTraceHook;
extern std::unique_ptr< VMTHook > g_pTEFireBullets;
extern std::unique_ptr< VMTHook > g_pNetShowFragments;
extern std::unique_ptr< VMTHook > g_pVGUIHook;
extern std::unique_ptr< VMTHook > g_pDirectHook;

typedef void(__thiscall* PaintTraverseFn)(void*, unsigned int, bool, bool);
typedef void(__stdcall* FrameStageNotifyFn)(ClientFrameStage_t);
typedef bool(__thiscall* CreateMoveFn)(void*, float, CUserCmd*);
typedef void(__fastcall* SceneEndFn)(void*, void*);
typedef bool(__thiscall* DoPostScreenEffectsFn)(void*, CViewSetup*);
typedef void(__thiscall* BeginFrameFn)(void*);
typedef bool(__thiscall* FireEventClientSide)(void*, IGameEvent*);
typedef void(__stdcall* OverRideViewFn)(void*);
typedef void(__thiscall* DrawModelExecuteFn)(void*, void*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix*);
typedef bool(__thiscall* SvCheatsGetBoolFn)(void*);
typedef float(__thiscall* AspectRatioFn)(void* pEcx, int32_t iWidth, int32_t iHeight);
typedef bool(__thiscall* InPredictionFn)(CPrediction*);
typedef void(__thiscall* RunCommandFn)(void*, IBasePlayer*, CUserCmd*, IMoveHelper*);
typedef float(__stdcall* GetViewModelFOVFn)();
typedef int(__thiscall* ListLeavesInBoxFn)(void*, const Vector&, const Vector&, unsigned short*, int);
typedef int32_t(__thiscall* BoxVisibleFn)(IEngineClient*, Vector&, Vector&);
typedef bool(__thiscall* IsHLTVFn)(IEngineClient*);
typedef bool(__thiscall* IsPausedFn)(void*);
typedef bool(__thiscall* WriteUsercmdDeltaToBufferFn) (void*, int, void*, int, int, bool);
typedef void(__thiscall* PacketStartFn)(void*, int, int);
typedef void(__thiscall* PacketEndFn)(void*);
typedef HRESULT(__stdcall* EndSceneFn)(IDirect3DDevice9* device);
typedef HRESULT(__stdcall* ResetFn)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

class CStudioHdr;
class CNetVarManager;

extern CInterfaces interfaces;
extern CNetVarManager netvars;
extern CCheatVars* csgo;

namespace I
{
	extern void Setup();
}

namespace F
{
	extern bool Shooting();
}

namespace H
{
	//extern ModifyEyePosFn ModifyEyePos;
	//extern AddViewModelBobFn AddViewModelBob;
	//extern CalcViewModelViewFn CalcViewModelView;
	//extern ClMoveFn ClMove;
	//extern SetupBonesFn SetupBones;
	//extern StandardBlendingRulesFn StandardBlendingRules;
	//extern ShouldSkipAnimFrameFn ShouldSkipAnimFrame;
	//extern DoExtraBoneProcessingFn DoExtraBoneProcessing;
	//extern UpdateClientSideAnimationFn UpdateClientSideAnimation;
	//extern PhysicsSimulateFn PhysicsSimulate;
	//extern PresentFn SteamPresent;
	//extern ResetFn SteamReset;
	//extern GetColorModulationFn GetColorModulation;
	//extern IsUsingStaticPropDebugModesFn IsUsingStaticPropDebugModes;

	void Hook();
	void UnHook();
}

extern void Msg(string str, color_t clr);

struct ResolverInfo_t;
