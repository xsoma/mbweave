#pragma once
#include "common.h"
#include "Vector.h"

struct ServerClass;
typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);
class IServerGameDLL
{
public:
	// Initialize the game (one-time call when the DLL is first loaded )
	// Return false if there is an error during startup.
	/*virtual bool			DLLInit(CreateInterfaceFn engineFactory,
										CreateInterfaceFn physicsFactory,
										CreateInterfaceFn fileSystemFactory,
											CGlobalVarsBase *pGlobals) = 0;*/

											// Setup replay interfaces on the server
	virtual bool			ReplayInit(CreateInterfaceFn fnReplayFactory) = 0;

	// This is called when a new game is started. (restart, map)
	virtual bool			GameInit(void) = 0;

	// Called any time a new level is started (after GameInit() also on level transitions within a game)
	virtual bool			LevelInit(char const *pMapName,
		char const *pMapEntities, char const *pOldLevel,
		char const *pLandmarkName, bool loadGame, bool background) = 0;

	// The server is about to activate
	virtual void			ServerActivate(void *pEdictList, int edictCount, int clientMax) = 0;

	// The server should run physics/think on all edicts
	virtual void			GameFrame(bool simulating) = 0;

	// Called once per simulation frame on the final tick
	virtual void			PreClientUpdate(bool simulating) = 0;

	// Called when a level is shutdown (including changing levels)
	virtual void			LevelShutdown(void) = 0;
	// This is called when a game ends (server disconnect, death, restart, load)
	// NOT on level transitions within a game
	virtual void			GameShutdown(void) = 0;

	// Called once during DLL shutdown
	virtual void			DLLShutdown(void) = 0;

	// Get the simulation interval (must be compiled with identical values into both client and game .dll for MOD!!!)
	// Right now this is only requested at server startup time so it can't be changed on the fly, etc.
	virtual float			GetTickInterval(void) const = 0;

	// Give the list of datatable classes to the engine.  The engine matches class names from here with
	//  edict_t::classname to figure out how to encode a class's data for networking
	virtual ServerClass*	GetAllServerClasses(void) = 0;

	// Returns string describing current .dll.  e.g., TeamFortress 2, Half-Life 2.  
	//  Hey, it's more descriptive than just the name of the game directory
	virtual const char     *GetGameDescription(void) = 0;

	// Let the game .dll allocate it's own network/shared string tables
	virtual void			CreateNetworkStringTables(void) = 0;

	/// not a complete class
};

class IClientRenderable;

struct ClientClass;

struct RenderableTranslucencyType_t;
struct RenderableModelType_t;
struct ClientRenderHandle_t;
class CClientLeafSubSystemData;
struct SetupRenderInfo_t;
struct ClientShadowHandle_t;
struct ClientLeafShadowHandle_t;

class IClientLeafSystem
{
public:
	//ClientLeafSystem002
	virtual void CreateRenderableHandle(IClientRenderable* pRenderable, bool bRenderWithViewModels, RenderableTranslucencyType_t nType, RenderableModelType_t nModelType, UINT32 nSplitscreenEnabled = 0xFFFFFFFF) = 0; // = RENDERABLE_MODEL_UNKNOWN_TYPE ) = 0;
	virtual void RemoveRenderable(ClientRenderHandle_t handle) = 0;
	virtual void AddRenderableToLeaves(ClientRenderHandle_t renderable, int nLeafCount, unsigned short* pLeaves) = 0;
	virtual void SetTranslucencyType(ClientRenderHandle_t handle, RenderableTranslucencyType_t nType) = 0;
	virtual void pad0() = 0;
	virtual void pad1() = 0;
	virtual void pad2() = 0;

	virtual void AddRenderable(IClientRenderable* pRenderable, bool IsStaticProp, RenderableTranslucencyType_t Type, RenderableModelType_t nModelType, UINT32 nSplitscreenEnabled = 0xFFFFFFFF) = 0; //7
	virtual bool IsRenderableInPVS(IClientRenderable* pRenderable) = 0; //8
	virtual void SetSubSystemDataInLeaf(int leaf, int nSubSystemIdx, CClientLeafSubSystemData* pData) = 0;
	virtual CClientLeafSubSystemData* GetSubSystemDataInLeaf(int leaf, int nSubSystemIdx) = 0;
	virtual void SetDetailObjectsInLeaf(int leaf, int firstDetailObject, int detailObjectCount) = 0;
	virtual void GetDetailObjectsInLeaf(int leaf, int& firstDetailObject, int& detailObjectCount) = 0;
	virtual void DrawDetailObjectsInLeaf(int leaf, int frameNumber, int& nFirstDetailObject, int& nDetailObjectCount) = 0;
	virtual bool ShouldDrawDetailObjectsInLeaf(int leaf, int frameNumber) = 0;
	virtual void RenderableChanged(ClientRenderHandle_t handle) = 0;
	virtual void BuildRenderablesList(const SetupRenderInfo_t& info) = 0;
	virtual void CollateViewModelRenderables(void*) = 0;
	virtual void DrawStaticProps(bool enable) = 0;
	virtual void DrawSmallEntities(bool enable) = 0;
	virtual ClientLeafShadowHandle_t AddShadow(ClientShadowHandle_t userId, unsigned short flags) = 0;
	virtual void RemoveShadow(ClientLeafShadowHandle_t h) = 0;
	virtual void ProjectShadow(ClientLeafShadowHandle_t handle, int nLeafCount, const int* pLeafList) = 0;
	virtual void ProjectFlashlight(ClientLeafShadowHandle_t handle, int nLeafCount, const int* pLeafList) = 0;
};

class CViewSetup {
public:
	int x, x_old;
	int y, y_old;
	int width, width_old;
	int height, height_old;
	bool m_bOrtho;
	float m_OrthoLeft;
	float m_OrthoTop;
	float m_OrthoRight;
	float m_OrthoBottom;
	bool m_bCustomViewMatrix;
	matrix m_matCustomViewMatrix;
	char pad_0x68[0x48];
	float fov;
	float fovViewmodel;
	Vector origin;
	Vector angles;
	float zNear;
	float zFar;
	float zNearViewmodel;
	float zFarViewmodel;
	float m_flAspectRatio;
	float m_flNearBlurDepth;
	float m_flNearFocusDepth;
	float m_flFarFocusDepth;
	float m_flFarBlurDepth;
	float m_flNearBlurRadius;
	float m_flFarBlurRadius;
	int m_nDoFQuality;
	int m_nMotionBlurMode;
	float m_flShutterTime;
	Vector m_vShutterOpenPosition;
	Vector m_shutterOpenAngles;
	Vector m_vShutterClosePosition;
	Vector m_shutterCloseAngles;
	float m_flOffCenterTop;
	float m_flOffCenterBottom;
	float m_flOffCenterLeft;
	float m_flOffCenterRight;
	int m_iEdgeBlur;
};

class IClientMode
{
public:

};

class IBaseClientDll
{
public:
	ClientClass* GetAllClasses()
	{
		typedef ClientClass* (__thiscall* OriginalFn)(PVOID);
		return getvfunc< OriginalFn >(this, 8)(this);
	}

	bool WriteUsercmdDeltaToBuffer(int nSlot, void* buf,
		int from, int to, bool isNewCmd)
	{
		using Fn = bool(__thiscall*)(void*, int, void*, int, int, bool);
		return getvfunc<Fn>(this, 24)(this, nSlot, buf, from, to, isNewCmd);
	}
};
