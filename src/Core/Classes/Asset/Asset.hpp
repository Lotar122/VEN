#pragma once

#include <cstdint>
#include "Classes/AssetUsage/AssetUsage.hpp"

namespace nihil
{
    namespace graphics { class Engine; };

    //Every asset which needs an ID (so every asset) needs to inherit from this class and call its base constructor
    ///The base class that you need to inherit from to get assetId and assetUsage
    class Asset
    {
    protected:
        ///The ID for an asset
        const uint32_t assetId = uint32_t(-1);
        ///The usage of an asset
        const AssetUsage assetUsage;
    public:
        ///The getter for assetId
        inline uint32_t _getAssetId() const { return assetId; };
        ///The getter for assetUsage
        inline AssetUsage _getAssetUsage() const { return assetUsage; };

        ///The ctor for Asset
        //
        ///@param _assetUsage The usage of an asset
        ///@param engine The pointer to the graphics::Engine
        Asset(AssetUsage _assetUsage, graphics::Engine* engine);
    };
}