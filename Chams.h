#pragma once
#include "Hooks.h"
#include <deque>

struct chams_t;

class CChams
{
private:

    IMaterial* Default;
    IMaterial* Flat;
    IMaterial* Glass;
    IMaterial* Metallic;

    IMaterial* GlowFade;
    IMaterial* GlowLine;
    IMaterial* Wireframe;

    struct CHitMatrixEntry {
        int ent_index;
        ModelRenderInfo_t info;
        DrawModelState_t state;
        matrix pBoneToWorld[128] = {};
        float time;
        matrix model_to_world;
    };
    std::deque<CHitMatrixEntry> m_Hitmatrix;

    IMaterial* GetMaterial(int index);

    bool DrawChams(const chams_t& chams_data, matrix* mat, bool ignore_z,
        DrawModelExecuteFn original,
        void* thisptr,
        void* ctx,
        const DrawModelState_t& state,
        const ModelRenderInfo_t& info
    );

public:
    void AddHitmatrix(animation* record);
    void OnPostScreenEffects();
    bool ShouldDraw(DrawModelExecuteFn original,
        void* thisptr,
        void* ctx,
        const DrawModelState_t& state,
        const ModelRenderInfo_t& info,
        matrix* pCustomBoneToWorld);
    void Init();
};