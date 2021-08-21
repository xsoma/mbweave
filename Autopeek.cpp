#include "Features.h"

void CAutopeek::GotoStart(CUserCmd* cur_cmd)
{
    if (!cur_cmd) {
        cur_cmd = csgo->cmd;
    }

    float wish_yaw = csgo->original.y;
    auto difference = csgo->local->GetRenderOrigin() - quickpeekstartpos;

    const auto chocked_ticks = (cur_cmd->command_number % 2) != 1 
        ? (14 - csgo->client_state->iChokedCommands) : csgo->client_state->iChokedCommands;

    static auto cl_forwardspeed = interfaces.cvars->FindVar(str("cl_forwardspeed"));

    if (difference.Length2D() > 5.0f)
    {
        auto angle = Math::CalculateAngle(csgo->local->GetRenderOrigin(), quickpeekstartpos);
        csgo->original.y = angle.y;

        cur_cmd->forwardmove = cl_forwardspeed->GetFloat() - (1.2f * chocked_ticks);
        cur_cmd->sidemove = 0.0f;
    }
    else {
        Reset();
    }
}

void CAutopeek::Draw()
{
    csgo->mtx.lock();
    if (!csgo->is_local_alive) {
        csgo->mtx.unlock();
        return;
    }

    if (!g_Binds[bind_peek_assist].active)
    {
        csgo->mtx.unlock();
        return;
    }

    if (quickpeekstartpos != Vector{ 0, 0, 0 })
    {
        static constexpr float Step = PI * 2.0f / 60;
        std::vector<ImVec2> points;
        for (float lat = 0.f; lat <= PI * 2.0f; lat += Step)
        {
            const auto& point3d = Vector(sin(lat), cos(lat), 0.f) * 25.f;
            Vector point2d;
            if (Math::WorldToScreen(quickpeekstartpos + point3d, point2d))
                points.push_back(ImVec2(point2d.x, point2d.y));
        }
        auto flags_backup = g_Render->_drawList->Flags;
        g_Render->_drawList->Flags |= ImDrawListFlags_AntiAliasedFill;
        g_Render->_drawList->AddConvexPolyFilled(points.data(), points.size(), color_t(20, 20, 20, 127).u32());
        g_Render->_drawList->AddPolyline(points.data(), points.size(), color_t(255, 255, 255, 127).u32(), true, 2.f);
        g_Render->_drawList->Flags = flags_backup;
    }
    csgo->mtx.unlock();
}

void CAutopeek::Run()
{
    if (!(csgo->local->GetFlags() & FL_ONGROUND))
        return;

    if (g_Binds[bind_peek_assist].active) {
        if (quickpeekstartpos == Vector(0, 0, 0))
            quickpeekstartpos = csgo->local->GetRenderOrigin();

        bool is_revolver = csgo->weapon->GetItemDefinitionIndex() == WEAPON_REVOLVER;

        if (!csgo->weapon->IsMiscWeapon() && !is_revolver && csgo->cmd->buttons & IN_ATTACK || is_revolver && g_Ragebot->shot)
            has_shot = true;

        if (has_shot)
            GotoStart();
    }
    else {
        Reset();
    }
}
