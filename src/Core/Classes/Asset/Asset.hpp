#pragma once

#include <cstdint>
#include "Classes/AssetUsage/AssetUsage.hpp"

namespace nihil
{
    namespace graphics { class Engine; };

    //Every asset which needs an ID (so every asset) needs to inherit from this class and call its base constructor
    class Asset
    {
    protected:
        const uint32_t assetId = uint32_t(-1);
        const AssetUsage assetUsage;
    public:
        inline uint32_t _getAssetId() const { return assetId; };
        inline AssetUsage _getAssetUsage() const { return assetUsage; };

        Asset(AssetUsage _assetUsage, graphics::Engine* engine);
    };
}