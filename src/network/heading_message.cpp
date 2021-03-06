#include "network/heading_message.h"
#include "core/i_move_component.h"
#include <portable_iarchive.hpp>
#include <portable_oarchive.hpp>
namespace network {

HeadingMessageSenderSystem::HeadingMessageSenderSystem()
    : MessageSenderSystem()
{

}

void HeadingMessageSenderSystem::Init()
{
    MessageSenderSystem::Init();
    SetFrequency( 10 );
    //        mSendHeadings.insert(platform::AutoId("player"));
    if ( mProgramState.mMode == ProgramState::Server )
    {
        mActorFrequencyTimerHolder.Add( ActorFrequencyTimer( 300.0, platform::AutoId( "spider1" ) ) );
        mActorFrequencyTimerHolder.Add( ActorFrequencyTimer( 300.0, platform::AutoId( "spider2" ) ) );
        mActorFrequencyTimerHolder.Add( ActorFrequencyTimer( 300.0, platform::AutoId( "spider1target" ) ) );
        mActorFrequencyTimerHolder.Add( ActorFrequencyTimer( 300.0, platform::AutoId( "spider2target" ) ) );
        mActorFrequencyTimerHolder.Add( ActorFrequencyTimer( 0.0, platform::AutoId( "player" ) ) );
        mActorFrequencyTimerHolder.Add( ActorFrequencyTimer( 0.0, platform::AutoId( "ctf_player" ) ) );
        mActorFrequencyTimerHolder.Add( ActorFrequencyTimer( 100.0, platform::AutoId( "grenade_projectile" ) ) );
        mActorFrequencyTimerHolder.Add( ActorFrequencyTimer( 100.0, platform::AutoId( "blue_grenade_projectile" ) ) );
        mActorFrequencyTimerHolder.Add( ActorFrequencyTimer( 300.0, platform::AutoId( "rocket_launcher_target_projectile" ) ) );
    }
    else if ( mProgramState.mMode == ProgramState::Client )
    {
        mActorFrequencyTimerHolder.Add( ActorFrequencyTimer( 0.0, platform::AutoId( "player" ) ) );
        mActorFrequencyTimerHolder.Add( ActorFrequencyTimer( 0.0, platform::AutoId( "ctf_player" ) ) );
    }
}

void HeadingMessageSenderSystem::Update( double DeltaTime )
{
    MessageSenderSystem::Update( DeltaTime );
    mActorFrequencyTimerHolder.Update( DeltaTime );
    if ( !IsTime() )
    {
        return;
    }
    if ( mProgramState.mMode == ProgramState::Server )
    {
        mSendHeadings = mActorFrequencyTimerHolder.GetActorIds();
        //TODO: might need optimization
        for ( ActorList_t::iterator it = mScene.GetActors().begin(), e = mScene.GetActors().end(); it != e; ++it )
        {
            Actor& actor = **it;
            if ( mSendHeadings.find( actor.GetId() ) == mSendHeadings.end() )
            {
                continue;
            }
            std::auto_ptr<HeadingMessage> HeadingMessage( GenerateHeadingMessage( actor ) );
            if ( HeadingMessage.get() != NULL )
            {
                mSingleMessageSender.Add( actor.GetGUID(), HeadingMessage );
            }
        }
    }
    else if ( mProgramState.mMode == ProgramState::Client )
    {
        Opt<Actor> player( mScene.GetActor( mProgramState.mControlledActorGUID ) );
        if ( player.IsValid() )
        {
            std::auto_ptr<HeadingMessage> HeadingMessage( GenerateHeadingMessage( *player ) );
            if ( HeadingMessage.get() != NULL )
            {
                mSingleMessageSender.Add( player->GetGUID(), HeadingMessage );
            }
        }
    }
}

std::auto_ptr<HeadingMessage> HeadingMessageSenderSystem::GenerateHeadingMessage( Actor& actor )
{
    Opt<IMoveComponent> moveC = actor.Get<IMoveComponent>();
    if ( !moveC.IsValid() )
    {
        return std::auto_ptr<HeadingMessage>();
    }
    std::auto_ptr<HeadingMessage> headingMsg( new HeadingMessage );
    headingMsg->mHeading = std::floor( moveC->GetHeading() * PRECISION );
    headingMsg->mActorGUID = actor.GetGUID();
    return headingMsg;
}

HeadingMessageHandlerSubSystem::HeadingMessageHandlerSubSystem()
    : MessageHandlerSubSystem()
{

}

void HeadingMessageHandlerSubSystem::Init()
{

}

void HeadingMessageHandlerSubSystem::Execute( Message const& message )
{
    HeadingMessage const& msg = static_cast<HeadingMessage const&>( message );

    if ( mProgramState.mMode == ProgramState::Server
         || ( mProgramState.mMode == ProgramState::Client && msg.mActorGUID != mProgramState.mControlledActorGUID ) )
    {
        Opt<Actor> actor = mScene.GetActor( msg.mActorGUID );
        if ( !actor.IsValid() )
        {
            L1( "cannot find actor with GUID: (that is not possible) %d \n", msg.mActorGUID );
            return;
        }

        Opt<IMoveComponent> moveC = actor->Get<IMoveComponent>();
        if ( !moveC.IsValid() )
        {
            L1( "heading is called on an actor that has no heading_component \n" );
            return;
        }
        moveC->SetHeading( msg.mHeading / PRECISION );
    }
}

} // namespace engine


REAPING2_CLASS_EXPORT_IMPLEMENT( network__HeadingMessage, network::HeadingMessage );
