#include "platform/i_platform.h"
#include "engine/health_system.h"
#include "core/collision_model.h"
#include "core/i_health_component.h"
#include "core/i_position_component.h"
#include "core/renderable_component.h"
#include "core/i_controller_component.h"
#include "core/i_move_component.h"
#include "core/damage_taken_event.h"

namespace engine {

HealthSystem::HealthSystem()
    : mScene( Scene::Get() )
{

}

void HealthSystem::Init()
{
}

void HealthSystem::Update(double DeltaTime)
{
    for( ActorList_t::iterator it = mScene.GetActors().begin(), e = mScene.GetActors().end(); it != e; ++it )
    {
        Actor& actor = **it;
        Opt<IHealthComponent> healthC=actor.Get<IHealthComponent>();
        if (!healthC.IsValid())
        {
            continue;
        }
        if (!healthC->IsAlive())
        {
            continue;
        }
        int32_t damage=healthC->GetDamage();


        int32_t currHp=healthC->GetHP();
        int32_t newHp=currHp-damage;

        if (newHp<currHp)
        {
            //TODO: thats not enough. This way walls bleed too.
            Opt<IPositionComponent> positionC = actor.Get<IPositionComponent>();
            if(positionC.IsValid())
            {
                EventServer<core::DamageTakenEvent>::Get().SendEvent( core::DamageTakenEvent( positionC->GetX(), positionC->GetY() ) );
            }
        }

        if (newHp<=0)
        {
            newHp=0;
            healthC->SetAlive(false);
            actor.Get<ICollisionComponent>()->SetCollisionClass( CollisionClass::No_Collision );
            healthC->SetTimeOfDeath(glfwGetTime());
            Scene::Get().ModifyActor(&actor,RenderableComponentModifier(RenderableLayer::Corpses,healthC->GetTimeOfDeath()));

            Opt<IControllerComponent> controllerC = actor.Get<IControllerComponent>();
            if(controllerC.IsValid())
            {
                controllerC->SetEnabled(false);
            }
            Opt<IMoveComponent> moveC=actor.Get<IMoveComponent>();
            if(moveC.IsValid())
            {
                moveC->SetSpeed(0);
            }
        }
        healthC->SetHP(newHp);
        healthC->ResetDamage();
    }

}

} // namespace engine
