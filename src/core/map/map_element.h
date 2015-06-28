#ifndef INCLUDED_CORE_MAP_MAP_ELEMENT_H
#define INCLUDED_CORE_MAP_MAP_ELEMENT_H

#include "platform/auto_id.h"
#include "core/opt.h"
#include "platform/jsonreader.h"
namespace map {

#define DEFINE_MAP_ELEMENT_BASE( BuffType ) \
    static int GetType_static() \
    { \
        static int const typ = platform::AutoId( #BuffType ); \
        return typ; \
    } \
    virtual int GetType() const \
    { \
        return BuffType::GetType_static(); \
    } \

class MapElement 
{
public:
    virtual int GetType() const=0;
    virtual ~MapElement();
    virtual void Load(Json::Value& setters)=0;
    int32_t GetUID();
protected:
    MapElement();
    int32_t mUID;  //does not need to be unique. Some elements does not need UID. Some elements can have the same UID.
};


class DefaultMapElement : public MapElement
{
public:
    DEFINE_MAP_ELEMENT_BASE(DefaultMapElement)
    DefaultMapElement();
    void Load(Json::Value& setters);
};
} // namespace map
#endif//INCLUDED_CORE_MAP_MAP_ELEMENT_H