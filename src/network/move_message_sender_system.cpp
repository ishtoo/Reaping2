#include "network/move_message_sender_system.h"
#include "core/i_move_component.h"
#include "move_message.h"
namespace network {


    MoveMessageSenderSystem::MoveMessageSenderSystem()
        : MessageSenderSystem()
    {

    }

    void MoveMessageSenderSystem::Init()
    {
        MessageSenderSystem::Init();
        SetFrequency(10);
        if (mProgramState.mMode == ProgramState::Server)
        {
            mSendMoves.insert(platform::AutoId("player"));
            mSendMoves.insert(platform::AutoId("ctf_player"));
            mSendMoves.insert(platform::AutoId("spider1"));
            mSendMoves.insert(platform::AutoId("spider2")); //tutucskaa :)
            mSendMoves.insert(platform::AutoId("spider1target"));
            mSendMoves.insert(platform::AutoId("spider2target"));
            mSendMoves.insert(platform::AutoId("rocket_launcher_target_projectile"));
            mSendMoves.insert(platform::AutoId("rusty_reaper_projectile"));
            mSendMoves.insert(platform::AutoId( "grenade_projectile" ) );
            mSendMoves.insert(platform::AutoId( "blue_grenade_projectile" ) );
         }
        else if (mProgramState.mMode == ProgramState::Client)
        {
            mSendMoves.insert(platform::AutoId("player"));
            mSendMoves.insert(platform::AutoId("ctf_player"));
        }
    }

    void MoveMessageSenderSystem::Update(double DeltaTime)
    {
        MessageSenderSystem::Update(DeltaTime);
        if (!IsTime())
        {
            return;
        }
        if (mProgramState.mMode == ProgramState::Server)
        {
            for (ActorList_t::iterator it = mScene.GetActors().begin(), e = mScene.GetActors().end(); it != e; ++it)
            {
                Actor& actor = **it;
                if (mSendMoves.find(actor.GetId()) == mSendMoves.end())
                {
                    continue;
                }
                std::auto_ptr<MoveMessage> moveMessage(GenerateMoveMessage(actor));
                if (moveMessage.get() != NULL)
                {
                    mSingleMessageSender.Add(actor.GetGUID(), moveMessage);
                }
            }
        }
        else if (mProgramState.mMode == ProgramState::Client)
        {
            Opt<Actor> player(mScene.GetActor(mProgramState.mControlledActorGUID));
            if (player.IsValid())
            {
                std::auto_ptr<MoveMessage> moveMessage(GenerateMoveMessage(*player));
                if (moveMessage.get() != NULL)
                {
                    mSingleMessageSender.Add(player->GetGUID(), moveMessage);
                }
            }
        }


}

std::auto_ptr<MoveMessage> MoveMessageSenderSystem::GenerateMoveMessage( Actor& actor )
{
    Opt<IMoveComponent> moveC = actor.Get<IMoveComponent>();
    if ( !moveC.IsValid() )
    {
        return std::auto_ptr<MoveMessage>();
    }
    std::auto_ptr<MoveMessage> moveMsg( new MoveMessage );
    moveMsg->mHeadingModifier = std::floor( moveC->GetHeadingModifier() * PRECISION );
    moveMsg->mSpeed = std::floor( moveC->GetSpeed().mBase.Get() * PRECISION );
    moveMsg->mPercent = std::floor( moveC->GetSpeed().mPercent.Get() * PRECISION );
    moveMsg->mFlat = std::floor( moveC->GetSpeed().mFlat.Get() * PRECISION );
    moveMsg->mMoving = moveC->GetMoving();
    moveMsg->mActorGUID = actor.GetGUID();
    moveMsg->mRooted = moveC->IsRooted();
    return moveMsg;
}

} // namespace engine

