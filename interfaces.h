#pragma once
#include "iBaseClientDLL.h"
#include "entlist.h"
#include "cvar.h"
#include "input_system.h"
#include "engine.h"
#include "engine_trace.h"
#include "surface.h"
#include "vpanel.h"
#include "global_vars.h"
#include "model_render.h"
#include "model_info.h"
#include "material_system.h"
#include "render_view.h"
#include "glow_manager.h"
#include "game_event.h"
#include "debug_overlay.h"
#include "studio_render.h"
#include "view_render_beams.h"
#include "physics_surface_props.h"
#include "c_movehelper.h"
#include "prediction.h"
#include "memalloc.h"

class c_model_interfaces
{
public:
	IVModelRender* model_render;
	IVModelInfo* model_info;
};

class CInterfaces
{
public:
	IServerGameDLL* server;
	IBaseClientDll* client;
	IClientMode* client_mode;
	CEntityList* ent_list;
	IClientLeafSystem* leaf_system;
	ICVar* cvars;
	IInputSystem* inputsystem;
	CInput* input;
	IEngineClient* engine;
	IEngineTrace* trace;
	CGlobalVars* global_vars;
	ISurface* surfaces;
	IEngineVGui* engine_vgui;
	IVPanel* v_panel;
	c_model_interfaces models;
	IMaterialSystem* material_system;
	IVRenderView* render_view;
	CGlowObjectManager* glow_manager;
	IGameEventManager2* event_manager;
	IVDebugOverlay* debug_overlay;
	IStudioRender* studio_render;
	IViewRenderBeams* beams;
	IPhysicsSurfaceProps* phys_props;
	IMoveHelper* move_helper;
	CGameMovement* game_movement;
	CPrediction* prediction;
	IMemAlloc* memalloc;
	IEngineSound* engine_sound;
	IMDLCache* model_cache;
	CNetworkStringTableContainer* net_string_container;
};

extern CInterfaces interfaces;