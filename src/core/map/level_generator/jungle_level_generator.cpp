#include "jungle_level_generator.h"
#include "platform/auto_id.h"
#include "../../i_position_component.h"
#include "random.h"
#include "level_generated_event.h"
#include "platform/settings.h"
#include <chrono>
#include <stack>

using platform::AutoId;

namespace map {

JungleLevelGenerator::JungleLevelGenerator( int32_t Id )
    : ILevelGenerator( Id )
    , mActorFactory( ActorFactory::Get() )
    , mRoomRepo( RoomRepo::Get() )
{
}

void JungleLevelGenerator::Generate()
{
    mRand.seed( mSeed == 0 ? unsigned( std::time( 0 ) ) : mSeed );
    int32_t a = mRand() % 10;
    mGData.SetDimensions( mCellCount, mCellCount );
    PlaceRooms( glm::vec2( 0, 0 ) );
    CreateStart();

    CreateMainRoute();
    CreateSideRoutes();

    GenerateTerrain();
    EventServer<LevelGeneratedEvent>::Get().SendEvent( LevelGeneratedEvent( LevelGeneratedEvent::TerrainGenerated ) );
}


void JungleLevelGenerator::Load( Json::Value& setters )
{
    ILevelGenerator::Load( setters );
    mMainRouteProperties.Load( setters["route"] );
    mSideRouteProperties.Load( setters["side_route"] );
    auto& possibleRooms = setters["possible_rooms"];
    if (possibleRooms.isArray())
    {
        for (auto& possibleRoom : possibleRooms)
        {
            std::string roomIdStr;
            int32_t chance;
            if (Json::GetStr( possibleRoom["room_id"], roomIdStr) && Json::GetInt( possibleRoom["weight"], chance ) )
            {
                AddPossibleRoom( AutoId( roomIdStr ), chance );
            }
        }
    }
}

void JungleLevelGenerator::CreateSideRoutes()
{
    int32_t roomIndex = 0;
    std::shuffle( mVisitedRooms.begin(), mVisitedRooms.end(), mRand );
    while (roomIndex < mVisitedRooms.size())
    {
        auto route = CreateRoute( mVisitedRooms[roomIndex], mSideRouteProperties );
        if (route.size() == 1)
        {
            ++roomIndex;
        }
        else
        {
            LinkRooms( route );
            roomIndex = 0;
            std::shuffle( mVisitedRooms.begin(), mVisitedRooms.end(), mRand );
        }

    }
}

void JungleLevelGenerator::CreateMainRoute()
{
    auto route = CreateRoute( mStartRoomIndex, mMainRouteProperties );
    LinkRooms( route );
    mEndRoomIndex = route.top();
    //TODO: check if it has an end property
    mGData.ClearRoomProperties( mEndRoomIndex );
    mGData.AddRoomProperty( mEndRoomIndex, RoomProperty::End );
}

void JungleLevelGenerator::GenerateTerrain()
{
    for (int32_t i = 0; i < mGData.GetRoomCount();++i)
    {
        mGData.GetRoom(i)->Generate(
            mGData.GetRoomDesc( i )
            , mGData.GetRoomCoord( i )*mCellSize + glm::vec2(mScene.GetDimensions()));
    }
}


void JungleLevelGenerator::RecreateBorders()
{

}

void JungleLevelGenerator::PlaceRooms( glm::vec2 const& startPos )
{
    FreeNodes_t freeCellPositions;
    freeCellPositions.push_back( startPos );
    while (!freeCellPositions.empty())
    {
        glm::vec2 cellPos = freeCellPositions.front();
        if (mGData.IsFilled( cellPos ))
        {
            LogNodes( "filled so pre_pop", freeCellPositions ); freeCellPositions.pop_front(); LogNodes( "filled so post_pop", freeCellPositions );
            continue;
        }
        Opt<IRoom> placedRoom = PlaceARoom( cellPos );
        if (placedRoom.IsValid())
        {
            LogNodes( "pre_pop", freeCellPositions ); freeCellPositions.pop_front(); LogNodes( "post_pop", freeCellPositions );
            auto& neighbours = placedRoom->GetNeighbourCells();
            for (auto& neighPos : neighbours)
            {
                auto possiblePos=neighPos + cellPos;
                if (mGData.IsInBounds( possiblePos )
                    &&!mGData.IsFilled( possiblePos )
                    &&std::find(freeCellPositions.begin(),freeCellPositions.end(),possiblePos)==freeCellPositions.end())
                {
                    freeCellPositions.push_back( possiblePos );
                }
            }
        }
        else
        {
            BOOST_ASSERT( false );
        }
    }
}

Opt<IRoom> JungleLevelGenerator::PlaceARoom( glm::vec2 const& pos )
{
    Opt<IRoom> r;
    std::shuffle( mPossibleRoomIds.begin(), mPossibleRoomIds.end(), mRand );
    bool succ = false;
    for (auto& roomId : mPossibleRoomIds)
    {
        auto& room = mRoomRepo( roomId );
        if (mGData.CanPlaceRoom( room.GetRoomDesc(), pos ))
        {
            mGData.PlaceRoom( room.GetRoomDesc(), pos );
            r = &room;
            break;
        }
    }
    return r;
}

void JungleLevelGenerator::LogNodes( std::string log, FreeNodes_t const& nodes )
{
    L2( "%d %s\n", nodes.size(), log.c_str() );
    for (auto& fn : nodes)
    {
        L2( "(%f,%f)", fn.x, fn.y );
    }
    L2( "\n" );
}


JungleLevelGenerator::Route_t JungleLevelGenerator::CreateRoute( int32_t startRoomIndex, RouteProperties const& properties )
{
    mGData.ShuffleNeighbours();
    Route_t route;
    int32_t currRoomIndex = startRoomIndex;
    route.push( currRoomIndex );
    Route_t longestRoute;
    Visited_t longestVisited;
    if (std::find( mVisitedRooms.begin(), mVisitedRooms.end(), currRoomIndex ) == mVisitedRooms.end())
    {
        mVisitedRooms.push_back( currRoomIndex );
    }
    std::vector<int32_t> nextNeigh(mGData.GetRoomCount(), 0);
    bool endHit = false;
    while (currRoomIndex != -1 && !endHit)
    {
        while (nextNeigh[currRoomIndex] < mGData.GetNeighbourRoomCount(currRoomIndex)
            &&std::find(mVisitedRooms.begin(), mVisitedRooms.end(),
                mGData.GetNeigbourRoomIndex(currRoomIndex, nextNeigh[currRoomIndex])) != mVisitedRooms.end())
        {
            ++nextNeigh[currRoomIndex];
        }
            
        if(nextNeigh[currRoomIndex] >= mGData.GetNeighbourRoomCount( currRoomIndex ))
        {
            // reached a dead end.
            if (properties.minLength == 0)
            {
                // if minLength is 0 then this means route creation is done
                endHit = true;
            }
            else
            {
                // stepping back
                // TODO: limit step backs in case of too long expected route size, this could be excessive
                nextNeigh[currRoomIndex] = 0;
                if (longestRoute.size() < route.size())
                {
                    // note that longest route and longest visited rooms have to be saved here
                    // the first room (startRoomIndex) have to stay in mVisited
                    longestRoute = route;
                    longestVisited = mVisitedRooms;
                }
                mVisitedRooms.erase( std::find( mVisitedRooms.begin(), mVisitedRooms.end(), currRoomIndex ) );
                route.pop();
                if (route.empty())
                {
                    currRoomIndex = -1;
                }
                else
                {
                    currRoomIndex = route.top();
                    ++nextNeigh[currRoomIndex];
                }
            }
        }
        else
        {
            // searching for a next neighbour
            int32_t nextRoomIndex = mGData.GetNeigbourRoomIndex( currRoomIndex, nextNeigh[currRoomIndex] );
            mVisitedRooms.push_back( nextRoomIndex );
            currRoomIndex = nextRoomIndex;
            route.push( currRoomIndex );
            if ((int32_t)route.size() - properties.minLength>0)
            {
                if (mRand() % (100) < properties.endChance + ((int32_t)route.size() - properties.minLength)*properties.chanceIncrease)
                {
                    // the desired route size has been hit
                    endHit = true;
                }
            }
        }
    }
    if (route.empty())
    {
        std::swap( mVisitedRooms, longestVisited );
        std::swap( longestRoute, route );
    }
    return route;
}


void JungleLevelGenerator::LinkRooms( Route_t route )
{
    int32_t roomB = route.top();
    route.pop();
    int32_t roomA = -1;
    while (!route.empty())
    {
        roomA = roomB;
        roomB = route.top();
        route.pop();
        auto cellPairs = mGData.GetAdjacentCellPairs( roomA, roomB );
        auto& cellPair = cellPairs[mRand() % cellPairs.size()];
        mGData.LinkCells( cellPair.first, cellPair.second );
    }
}

void JungleLevelGenerator::CreateStart()
{
    mStartRoomIndex = mRand() % mGData.GetRoomCount();
    //TODO: check if it has a start property
    mGData.ClearRoomProperties( mStartRoomIndex );
    mGData.AddRoomProperty( mStartRoomIndex, RoomProperty::Start );
}


void JungleLevelGenerator::RouteProperties::Load( Json::Value& setters )
{
    Json::GetInt( setters["min_length"], minLength );
    Json::GetInt( setters["end_chance"], endChance );
    Json::GetInt( setters["chance_increase"], chanceIncrease );
}

} // namespace map

