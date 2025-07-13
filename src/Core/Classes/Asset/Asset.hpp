#pragma once

namespace nihil
{
    //Every asset which need an ID (so every asset) needs to inherit from this class and call its base constructor
    class Asset
    {
    protected:
        uint64_t assetId = uint64_t(-1);
    public:
        inline const uint64_t _getAssetId() const { return assetId; };

        Asset(Engine* engine)
        {
            assetId = engine->requestAssetId();
        }
    };
}