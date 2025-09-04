#include "Asset.hpp"

#include "Classes/Engine/Engine.hpp"

using namespace nihil;

Asset::Asset(AssetUsage _assetUsage, graphics::Engine* engine) : assetId(engine->requestAssetId()), assetUsage(_assetUsage)
{
    assert(engine != nullptr);
}