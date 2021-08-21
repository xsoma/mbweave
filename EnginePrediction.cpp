#include "EnginePrediction.h"
#include "checksum_md5.h"
#include "AntiAims.h"
#include "netvar_manager.h"

void CNetData::Store() {
    if (!csgo->local && !csgo->local->isAlive()) {
        Reset();
        return;
    }

    int          tickbase;
    StoredData_t* data;

    tickbase = csgo->local->GetTickBase();

    // get current record and store data.
    data = &m_data[tickbase % 150];

    data->m_tickbase = tickbase;
    data->m_punch = csgo->local->GetPunchAngle();
    data->m_punch_vel = csgo->local->GetPunchAngleVel();
    data->m_view_offset = csgo->local->GetVecViewOffset();
    data->m_velocity_modifier = csgo->local->GetVelocityModifier();
}

void CNetData::Apply() {
    if (!csgo->local && !csgo->local->isAlive()) {
        Reset();
        return;
    }

    int          tickbase;
    StoredData_t* data;
    Vector        punch_delta, punch_vel_delta;
    Vector       view_delta;
    float        modifier_delta;

    tickbase = csgo->local->GetTickBase();

    // get current record and validate.
    data = &m_data[tickbase % 150];

    if (csgo->local->GetTickBase() != data->m_tickbase)
        return;

    // get deltas.
    // note - dex;  before, when you stop shooting, punch values would sit around 0.03125 and then goto 0 next update.
    //              with this fix applied, values slowly decay under 0.03125.
    punch_delta = csgo->local->GetPunchAngle() - data->m_punch;
    punch_vel_delta = csgo->local->GetPunchAngleVel() - data->m_punch_vel;
    view_delta = csgo->local->GetVecViewOffset() - data->m_view_offset;
    modifier_delta = csgo->local->GetVelocityModifier() - data->m_velocity_modifier;

    // set data.
    if (std::abs(punch_delta.x) < 0.03125f &&
        std::abs(punch_delta.y) < 0.03125f &&
        std::abs(punch_delta.z) < 0.03125f)
        csgo->local->GetPunchAngle() = data->m_punch;

    if (std::abs(punch_vel_delta.x) < 0.03125f &&
        std::abs(punch_vel_delta.y) < 0.03125f &&
        std::abs(punch_vel_delta.z) < 0.03125f)
        csgo->local->GetPunchAngleVel() = data->m_punch_vel;

    if (std::abs(view_delta.x) < 0.03125f &&
        std::abs(view_delta.y) < 0.03125f &&
        std::abs(view_delta.z) < 0.03125f)
        csgo->local->GetVecViewOffset() = data->m_view_offset;

    if (std::abs(modifier_delta) < 0.03125f)
        csgo->local->GetVelocityModifier() = data->m_velocity_modifier;
}

void CNetData::Reset() {
    m_data.fill(StoredData_t());
}

void CNetData::RecordViewmodelValues()
{
    this->viewModelData.m_hWeapon = 0;
    auto viewmodel = csgo->local->GetViewModel();
    if (viewmodel) {
        this->viewModelData.m_hWeapon = viewmodel->GetViewmodelWeapon();
        this->viewModelData.m_nViewModelIndex = viewmodel->GetViewModelIndex();
        this->viewModelData.m_nSequence = viewmodel->GetCurrentSequence();

        this->viewModelData.networkedCycle = viewmodel->GetCurrentCycle();
        this->viewModelData.m_nAnimationParity = viewmodel->GetAnimationParity();
        this->viewModelData.animationTime = viewmodel->GetModelAnimTime();
    }
}

void CNetData::ApplyViewmodelValues()
{
    auto viewmodel = csgo->local->GetViewModel();
    if (viewmodel) {
        if (this->viewModelData.m_nSequence == viewmodel->GetCurrentSequence()
            && this->viewModelData.m_hWeapon == viewmodel->GetViewmodelWeapon()
            && this->viewModelData.m_nAnimationParity == viewmodel->GetAnimationParity()) {
            viewmodel->GetCurrentCycle() = this->viewModelData.networkedCycle;
            viewmodel->GetModelAnimTime() = this->viewModelData.animationTime;
        }
    }
}