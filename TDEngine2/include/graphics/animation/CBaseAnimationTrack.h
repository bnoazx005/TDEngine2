/*!
	\file CBaseAnimationTrack.h
	\date 21.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "IAnimationTrack.h"
#include "../../core/CBaseObject.h"
#include "../../math/MathUtils.h"
#include <vector>
#include <string>
#include <algorithm>


namespace TDEngine2
{
	/*!
		class CBaseAnimationTrack

		\brief All other track types should be implemented based on this
	*/

	template <typename TKeyFrameType>
	class CBaseAnimationTrack : public IAnimationTrack, public CBaseObject
	{
		public:
			typedef std::vector<TKeyFrameType> TKeysArray;
		public:
			virtual ~CBaseAnimationTrack() = default;

			TAnimationTrackKeyId CreateKey(F32 time)
			{
				auto it = std::find_if(mKeys.cbegin(), mKeys.cend(), [time](const TKeyFrameType& key) { return CMathUtils::Abs(key.mTime - time) < 1e-3f; });
				if (it != mKeys.cend()) 
				{
					return static_cast<TAnimationTrackKeyId>(std::distance(mKeys.cbegin(), it));
				}

				TAnimationTrackKeyId handle = static_cast<TAnimationTrackKeyId>(mKeys.size());

				mKeys.emplace_back(time, {});

				return handle;
			}

			E_RESULT_CODE RemoveKey(TAnimationTrackKeyId handle)
			{
				if (handle == TAnimationTrackKeyId::Invalid || static_cast<size_t>(handle) >= mKeys.size())
				{
					return RC_INVALID_ARGS;
				}

				mKeys.erase(mKeys.cbegin() + static_cast<size_t>(handle));

				return RC_OK;
			}

#if 0
			E_RESULT_CODE Apply(const TAnimatedValueWrapper<T>& wrapper, F32 time)
			{
				// \todo implement this one
				return RC_OK;
			}
#endif

			TKeyFrameType* GetKey(TAnimationTrackKeyId handle) const
			{
				if (handle == TAnimationTrackKeyId::Invalid || static_cast<size_t>(handle) >= mKeys.size())
				{
					return nullptr;
				}

				return mKeys[static_cast<U32>(handle)];
			}

		protected:
			std::string mName;

			TKeysArray mKeys;
	};
}