#include "Asset.hpp"

#include "Classes/Engine/Engine.hpp"

using namespace nihil;

Asset::Asset(graphics::Engine* engine)
{
    assert(engine != nullptr);
    assetId = engine->requestAssetId();
}