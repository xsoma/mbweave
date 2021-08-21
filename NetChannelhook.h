#pragma once
#include "Hooks.h"
#include "iBaseClientDLL.h"
#include "EnginePrediction.h"
#include "Dormant.h"

void __fastcall Hooked_PacketStart(void* ecx, void*, int incoming, int outgoing) {
    static auto original_fn = g_pClientStateAdd->GetOriginal<PacketStartFn>(g_HookIndices[fnva1(hs::Hooked_PacketStart.s().c_str())]);

    if (csgo->DoUnload)
        return original_fn(ecx, incoming, outgoing);

    if (!csgo->local->isAlive())
        return original_fn(ecx, incoming, outgoing);

    if (csgo->packets.empty())
        return original_fn(ecx, incoming, outgoing);

    for (auto it = csgo->packets.rbegin(); it != csgo->packets.rend(); ++it)
    {
        if (!it->is_outgoing)
            continue;

        if (it->cmd_number == outgoing || outgoing > it->cmd_number && (!it->is_used || it->previous_command_number == outgoing))
        {
            it->previous_command_number = outgoing;
            it->is_used = true;
            original_fn(ecx, incoming, outgoing);
            break;
        }
    }

    auto result = false;

    for (auto it = csgo->packets.begin(); it != csgo->packets.end();)
    {
        if (outgoing == it->cmd_number || outgoing == it->previous_command_number)
            result = true;

        if (outgoing > it->cmd_number && outgoing > it->previous_command_number)
            it = csgo->packets.erase(it);
        else
            ++it;
    }

    if (!result)
        original_fn(ecx, incoming, outgoing);
}

void __fastcall Hooked_PacketEnd(void* ecx, void* edx)
{
    static auto original_fn = g_pClientStateAdd->GetOriginal <PacketEndFn>(g_HookIndices[fnva1(hs::Hooked_PacketEnd.s().c_str())]);

    if (!csgo->local->isAlive())  //-V807
    {
        csgo->c_data.clear();
        return original_fn(ecx);
    }
    auto clientstate = (CClientState*)ecx;
    if (*(int*)((uintptr_t)ecx + 0x164) == *(int*)((uintptr_t)ecx + 0x16C))
    {
        auto ack_cmd = *(int*)((uintptr_t)ecx + 0x4D2C);
        auto correct = std::find_if(csgo->c_data.begin(), csgo->c_data.end(),
            [&ack_cmd](const correction_data& other_data)
            {
                return other_data.command_number == ack_cmd;
            }
        );

        auto netchannel = interfaces.engine->GetNetChannelInfo();

        if (netchannel && correct != csgo->c_data.end())
        {
            if (csgo->velocity_modifier > csgo->local->GetVelocityModifier() + 0.1f)
            {
                auto weapon = csgo->weapon;

                if (weapon || weapon->GetItemDefinitionIndex() != WEAPON_REVOLVER && !weapon->IsNade()) //-V648
                {
                    for (auto& number : csgo->choked_number)
                    {
                        auto cmd = &interfaces.input->m_pCommands[number % 150];
                        auto verified = &interfaces.input->m_pVerifiedCommands[number % 150];

                        if (cmd->buttons & (IN_ATTACK | IN_ATTACK2))
                        {
                            cmd->buttons &= ~IN_ATTACK;

                            verified->m_cmd = *cmd;
                            verified->m_crc = cmd->GetChecksum();
                        }
                    }
                }
            }

            csgo->velocity_modifier = csgo->local->GetVelocityModifier();
        }
    }

    return original_fn(ecx);
}