#include "settings.h"

Settings::Settings()
{
    Init();
}

namespace {
Json::Value parseFile( AutoFile& F )
{
    if (!F.get() || !F->IsValid())
    {
        return Json::Value();
    }
    JsonReader Reader(*F);
    if (!Reader.IsValid())
    {
        return Json::Value();
    }
    return Reader.GetRoot();
}
Json::Value getMountedSettings()
{
    Filesys& Fsys(Filesys::Get());
    AutoFile F = Fsys.Open(boost::filesystem::path("settings.json"));
    return parseFile( F );
}
Json::Value getUserSettings()
{
    AutoFile userSettings( new OsFile( "settings.json" ) );
    return parseFile( userSettings );
}
}

void Settings::Init()
{
    Json::Value settings;
    // user settings first, defaults later
    settings = getUserSettings();
    if( !!settings )
    {
        mRoots.push_back( settings );
    }
    settings = getMountedSettings();
    if( !!settings )
    {
        mRoots.push_back( settings );
    }
}

std::string Settings::GetStr(std::string const& key, std::string const& Default/*=""*/) const
{
    std::string r=Default;
    Json::GetStr(Resolve(key), r);
    return r;
}

uint32_t Settings::GetUInt(std::string const& key, uint32_t Default /*= 0*/) const
{
    Json::GetUInt(Resolve(key), Default);
    return Default;
}

int32_t Settings::GetInt(std::string const& key, int32_t Default /*= 0*/) const
{
    Json::GetInt(Resolve(key), Default);
    return Default;
}

double Settings::GetDouble(std::string const& key, double Default /*= 0.0*/) const
{
    Json::GetDouble(Resolve(key), Default);
    return Default;
}

float Settings::GetFloat(std::string const& key, float Default /*= 0.0f*/) const
{
    Json::GetFloat(Resolve(key), Default);
    return Default;
}

int32_t Settings::GetColor(std::string const& key, int32_t Default /*= 0.0f*/) const
{
    Json::GetColor(Resolve(key), Default);
    return Default;
}

glm::vec4 Settings::GetColor(std::string const& key, glm::vec4 Default /*= glm::vec4(0.0)*/) const
{
    Json::GetColor(Resolve(key), Default);
    return Default;
}

Json::Value Settings::Resolve(std::string const& key) const
{
    std::vector<std::string> keySplit;
    boost::split(keySplit, key, boost::is_any_of("."));
    Json::Value r;
    for( auto& root : mRoots )
    {
        r = root;
        for (auto const& i : keySplit)
        {
            r = r[i];
        }
        // return the first valid found setting
        // note: not type-sensitive
        if( !!r )
        {
            break;
        }
    }
    return r;
}

