#pragma once

#include <cstdint>

namespace nihil
{
    namespace graphics { class Engine; };

    //Every asset which need an ID (so every asset) needs to inherit from this class and call its base constructor
    class Asset
    {
    protected:
        uint32_t assetId = uint32_t(-1);
    public:
        inline uint32_t _getAssetId() const { return assetId; };

        Asset(graphics::Engine* engine);
    };
}