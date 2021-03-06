#ifndef INCLUDED_ENGINE_ITEMS_GATLING_GUN_WEAPON_SUB_SYSTEM_H
#define INCLUDED_ENGINE_ITEMS_GATLING_GUN_WEAPON_SUB_SYSTEM_H

#include "engine/items/common_sub_system_includes.h"

namespace engine {

class GatlingGunWeaponSubSystem : public SubSystem, public SubSystemHolder
{
public:
    DEFINE_SUB_SYSTEM_BASE( GatlingGunWeaponSubSystem )

    GatlingGunWeaponSubSystem();
    virtual void Init();
    virtual void Update( Actor& actor, double DeltaTime );
private:
    Scene& mScene;
    core::ProgramState& mProgramState;
    Opt<WeaponItemSubSystem> mWeaponItemSubSystem;
    ActorFactory& mActorFactory;
    int32_t mShotId;
};

} // namespace engine

#endif//INCLUDED_ENGINE_ITEMS_GATLING_GUN_WEAPON_SUB_SYSTEM_H
