#include "../../include/graphics/CGraphicsLayersInfo.h"
#include <climits>
#include <cmath>


namespace TDEngine2
{
	CGraphicsLayersInfo::CGraphicsLayersInfo():
		CBaseObject()
	{
	}

	E_RESULT_CODE CGraphicsLayersInfo::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CGraphicsLayersInfo::AddLayer(F32 depthValue, const C8* layerName)
	{
		/// if there is no any layer within the vector use -infinity as the most left value
		F32 prevLayerDepthValue = mGraphicsLayers.empty() ? -(std::numeric_limits<F32>::infinity)() : std::get<F32>(mGraphicsLayers.back()); 

		if (mGraphicsLayers.empty())
		{
			prevLayerDepthValue = -(std::numeric_limits<F32>::infinity)(); /// a left bound
		}

		if (depthValue < prevLayerDepthValue) /// all layers should be defined from smallest values to highest one
		{
			return RC_INVALID_ARGS;
		}

		mGraphicsLayers.emplace_back(depthValue, (layerName ? layerName : "GraphicsLayer" + mGraphicsLayers.size()));

		return RC_OK;
	}

	U16 CGraphicsLayersInfo::GetLayerIndex(F32 depthValue) const
	{
		if (mGraphicsLayers.empty())
		{
			return 0; /// there is the only one layer, so return it
		}

		U16 layerId = 0;

		F32 rightLayerBorderValue = 0.0f;

		for (TGraphicsLayersArray::const_iterator iter = mGraphicsLayers.cbegin(); iter != mGraphicsLayers.cend(); ++iter, ++layerId)
		{
			rightLayerBorderValue = std::get<F32>(*iter);

			if (depthValue < rightLayerBorderValue || fabs(depthValue - rightLayerBorderValue) < FloatEpsilon)
			{
				return layerId;
			}
		}

		return layerId;
	}


	IGraphicsLayersInfo* CreateGraphicsLayersInfo(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsLayersInfo, CGraphicsLayersInfo, result);
	}
}