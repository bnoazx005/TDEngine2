#include "../../include/editor/EditorUtils.h"
#include "../../include/graphics/IDebugUtility.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	void DrawEditorGrid(IDebugUtility* pDebugUtility, I32 rows, I32 cols, F32 cellSize)
	{
		const F32 halfWidth = 0.5f * cols * cellSize;
		const F32 halfHeight = 0.5f * rows * cellSize;

		for (I32 i = rows / 2; i >= -rows / 2; --i)
		{
			if (!i)
			{
				continue;
			}

			pDebugUtility->DrawLine(TVector3(-halfWidth, 0.0f, i * cellSize), TVector3(halfWidth, 0.0f, i * cellSize), TColorUtils::mWhite);
		}

		for (I32 i = cols / 2; i >= -cols / 2; --i)
		{
			if (!i)
			{
				continue;
			}

			pDebugUtility->DrawLine(TVector3(i * cellSize, 0.0f, halfHeight), TVector3(i * cellSize, 0.0f, -halfHeight), TColorUtils::mWhite);
		}

		/// \note Center lines

		pDebugUtility->DrawLine(TVector3(-halfWidth, 0.0f, 0.0f), TVector3(halfWidth, 0.0f, 0.0f), TColorUtils::mRed);
		pDebugUtility->DrawLine(TVector3(0.0f, 0.0f, -halfHeight), TVector3(0.0f, 0.0f, halfHeight), TColorUtils::mRed);
	}

}

#endif