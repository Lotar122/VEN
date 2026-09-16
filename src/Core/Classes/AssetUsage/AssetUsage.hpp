#pragma once

namespace nihil
{
	///The enum for how an asset is used
	enum class AssetUsage
	{
		///Never updated
		Static,
		///Can be updated
		Dynamic,
		///Unused
		Undefined
	};
}