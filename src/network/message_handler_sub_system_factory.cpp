#include "platform/i_platform.h"
#include "network/message_handler_sub_system_factory.h"
#include "platform/auto_id.h"
#include "my_name_message_handler_sub_system.h"
#include "client_id_message_handler_sub_system.h"
#include "lifecycle_message_handler_sub_system.h"
#include "create_actor_message_handler_sub_system.h"
#include "set_ownership_message_handler_sub_system.h"
#include "position_message_handler_sub_system.h"

using platform::AutoId;
namespace network {

MessageHandlerSubSystemFactory::MessageHandlerSubSystemFactory()
{
    Bind( AutoId("default_message_handler_sub_system"), &CreateSubSystem<DefaultMessageHandlerSubSystem> );
    SetDefault( AutoId("default_message_handler_sub_system") );

    Bind( AutoId("my_name_message_handler_sub_system"), &CreateSubSystem<MyNameMessageHandlerSubSystem> );
    Bind( AutoId("client_id_message_handler_sub_system"), &CreateSubSystem<ClientIdMessageHandlerSubSystem> );
    Bind( AutoId("lifecycle_message_handler_sub_system"), &CreateSubSystem<LifecycleMessageHandlerSubSystem> );
    Bind( AutoId("create_actor_message_handler_sub_system"), &CreateSubSystem<CreateActorMessageHandlerSubSystem> );
    Bind( AutoId("set_ownership_message_handler_sub_system"), &CreateSubSystem<SetOwnershipMessageHandlerSubSystem> );
    Bind( AutoId("position_message_handler_sub_system"), &CreateSubSystem<PositionMessageHandlerSubSystem> );
}

} // namespace engine
