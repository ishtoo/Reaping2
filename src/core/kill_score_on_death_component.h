#ifndef INCLUDED_CORE_KILL_SCORE_ON_DEATH_COMPONENT_H
#define INCLUDED_CORE_KILL_SCORE_ON_DEATH_COMPONENT_H

#include "i_kill_score_on_death_component.h"
#include "core/property_loader.h"

class KillScoreOnDeathComponent : public IKillScoreOnDeathComponent
{
public:
    KillScoreOnDeathComponent();
    virtual void SetKillerGUID(int32_t killerGUID);
    virtual int32_t GetKillerGUID()const;
    virtual void SetDeadGUID(int32_t deadGUID);
    virtual int32_t GetDeadGUID()const;
    virtual void SetScored(bool scored);
    virtual bool IsScored()const;
protected:
    friend class ComponentFactory;
    int32_t mKillerGUID;
    int32_t mDeadGUID;
    bool mScored;
private:
};

class KillScoreOnDeathComponentLoader : public ComponentLoader<KillScoreOnDeathComponent>
{
    virtual void BindValues();
protected:
    KillScoreOnDeathComponentLoader();
    friend class ComponentLoaderFactory;
};

#endif//INCLUDED_CORE_KILL_SCORE_ON_DEATH_COMPONENT_H

//command:  "classgenerator.exe" -g "component" -c "kill_score_on_death_component" -m "int32_t-killerGUID int32_t-deadGUID bool-scored"