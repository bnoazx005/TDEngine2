/*!
	\file CBaseAnimationTrack.h
	\date 21.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "IAnimationTrack.h"
#include "IAnimationClip.h"
#include "../../core/CBaseObject.h"
#include "../../math/MathUtils.h"
#include "stringUtils.hpp"
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <unordered_map>


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
			typedef std::unordered_map<TAnimationTrackKeyId, U32> TKeysHandleRegistry;
		public:
			/*!
				\brief The method initializes an internal state of an animation track

				\param[in, out] pTrackOwner A pointer to IAnimationClip which holds the track

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IAnimationClip* pTrackOwner) override
			{
				if (mIsInitialized)
				{
					return RC_FAIL;
				}

				if (!pTrackOwner)
				{
					return RC_INVALID_ARGS;
				}

				mpTrackOwnerAnimation = pTrackOwner;
				mInterpolationMode = E_ANIMATION_INTERPOLATION_MODE_TYPE::CUBIC;

				mIsInitialized = true;

				return RC_OK;
			}

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override
			{
				if (!pReader)
				{
					return RC_INVALID_ARGS;
				}

				mName = pReader->GetString(mNameKeyId);
				mPropertyBinding = pReader->GetString(mBindingKeyId);
				mInterpolationMode = static_cast<E_ANIMATION_INTERPOLATION_MODE_TYPE>(pReader->GetUInt16(mInterpolationModeKeyId));

				pReader->BeginGroup("keys");

				while (pReader->HasNextItem())
				{
					pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
					{
						pReader->BeginGroup("key");
						{
							U8 usedChannels = pReader->GetUInt8("used_channels_mask");
							usedChannels = usedChannels ? usedChannels : static_cast<U8>(0xFF);

							_loadKeyFrameValue(CreateKey(pReader->GetFloat("time"), &usedChannels), pReader);
						}
						pReader->EndGroup();
					}
					pReader->EndGroup();
				}

				pReader->EndGroup();

				return RC_OK;
			}

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override
			{
				if (!pWriter)
				{
					return RC_INVALID_ARGS;
				}

				pWriter->SetString(mNameKeyId, mName);
				pWriter->SetString(mBindingKeyId, mPropertyBinding);
				pWriter->SetUInt16(mInterpolationModeKeyId, static_cast<U16>(mInterpolationMode));

				pWriter->BeginGroup("keys", true);

				for (const TKeyFrameType& currKey : mKeys)
				{
					pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
					{
						pWriter->BeginGroup("key");

						pWriter->SetFloat("time", currKey.mTime);
						pWriter->SetUInt8("used_channels_mask", currKey.mUsedChannels);
						_saveKeyFrameValue(currKey, pWriter); // \note Write down the value

						pWriter->EndGroup();
					}
					pWriter->EndGroup();
				}

				pWriter->EndGroup();

				return RC_OK;
			}

			/*!
				\brief The method creates a new key frame in the track. Each invokation of the method should save
				keys in sorted manner
			*/

			TDE2_API TAnimationTrackKeyId CreateKey(F32 time, const U8* pChannelsUsageMask = nullptr) override
			{
				auto it = std::find_if(mKeys.begin(), mKeys.end(), [time](const TKeyFrameType& key) { return CMathUtils::Abs(key.mTime - time) < 1e-3f; });
				if (it != mKeys.end()) 
				{
					if (pChannelsUsageMask)
					{
						it->mUsedChannels = *pChannelsUsageMask; /// Update current used channels 
					}

					return GetKeyHandleByTime(time);
				}

				TAnimationTrackKeyId handle = static_cast<TAnimationTrackKeyId>(mKeys.size());

				it = std::find_if(mKeys.begin(), mKeys.end(), [time](const TKeyFrameType& key) { return key.mTime > time; }); // find first key which is lesser than a new one

				const U32 index = static_cast<U32>(std::distance(mKeys.begin(), it));

				TKeyFrameType newFrameValue;

				newFrameValue.mTime = time;
				newFrameValue.mUsedChannels = pChannelsUsageMask ? *pChannelsUsageMask : static_cast<U8>(0xFF);

				mKeys.insert(it, std::move(newFrameValue));

				// \note refresh handles
				for (auto& currHandleEntity : mKeysHandlesMap)
				{
					if (currHandleEntity.second >= index)
					{
						++currHandleEntity.second;
					}
				}

				mKeysHandlesMap.insert({ handle, index });

				return handle;
			}

			TDE2_API E_RESULT_CODE RemoveKey(TAnimationTrackKeyId handle) override
			{
				auto it = mKeysHandlesMap.find(handle);

				if (handle == TAnimationTrackKeyId::Invalid || it == mKeysHandlesMap.cend())
				{
					return RC_INVALID_ARGS;
				}

				const U32 index = it->second;

				mKeys.erase(mKeys.cbegin() + index);
				mKeysHandlesMap.erase(it);

				// \note refresh handles
				for (auto& currHandleEntity : mKeysHandlesMap)
				{
					if (currHandleEntity.second > index)
					{
						--currHandleEntity.second;
					}
				}

				return RC_OK;
			}

			TDE2_API void RemoveAllKeys() override
			{
				mKeysHandlesMap.clear();
				mKeys.clear();
			}

			/*!
				\brief The method specifies interpolation mode for tracks

				\param[in] mode The value determines sampling type
			*/

			TDE2_API void SetInterpolationMode(E_ANIMATION_INTERPOLATION_MODE_TYPE mode) override
			{
				mInterpolationMode = mode;
			}

			/*!
				\brief The method assign a string which contains a name of property that will be animated
				by this track

				\param[in] binding Current format looks like the following component_name.property_name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetPropertyBinding(const std::string& binding) override
			{
				if (binding.empty())
				{
					return RC_INVALID_ARGS;
				}

				mPropertyBinding = binding;

				return RC_OK;
			}

			TDE2_API E_RESULT_CODE SetName(const std::string& name) override
			{
				if (name.empty())
				{
					return RC_INVALID_ARGS;
				}

				mName = name;

				return RC_FAIL;
			}

#if TDE2_EDITORS_ENABLED

			TDE2_API E_RESULT_CODE UpdateKeyTime(TAnimationTrackKeyId keyId, F32 value, bool reorderKeys = false) override
			{
				if (auto pKey = GetKey(keyId))
				{
					pKey->mTime = value;

					const U32 oldIndex = mKeysHandlesMap[keyId];
					/// find first key which is lesser than a new one
					U32 newIndex = static_cast<U32>(std::distance(mKeys.cbegin(), std::find_if(mKeys.cbegin(), mKeys.cend(), [value](const TKeyFrameType& key) { return key.mTime > value; })));

					/// \note If the existing one index is the same with new one skip reordering ( subtract 1 because of a new value of the key always will be greater than its previous one )
					if ((oldIndex == newIndex - 1) || (mKeys.size() < 2))
					{
						return RC_OK;
					}

					newIndex = (newIndex - 1) >= mKeys.size() ? 0 : newIndex - 1;

					/// \note Update order of the keys
					const U32 first  = std::min<U32>(oldIndex, newIndex);
					const U32 second = std::max<U32>(oldIndex, newIndex);

					/// \note Swap two elements including current one
					TKeyFrameType tmp = mKeys[first];
					mKeys[first] = mKeys[second];
					mKeys[second] = tmp;

					/// \todo Update indices in the hash table
					auto it0 = std::find_if(mKeysHandlesMap.begin(), mKeysHandlesMap.end(), [first](auto&& entity) { return entity.second == first; });
					auto it1 = std::find_if(mKeysHandlesMap.begin(), mKeysHandlesMap.end(), [second](auto&& entity) { return entity.second == second; });

					it0->second = second;
					it1->second = first;					

					return RC_OK;
				}

				return RC_FAIL;
			}

#endif

			TKeyFrameType* GetKey(TAnimationTrackKeyId handle)
			{
				auto iter = mKeysHandlesMap.find(handle);

				if (handle == TAnimationTrackKeyId::Invalid || iter == mKeysHandlesMap.cend())
				{
					return nullptr;
				}

				return &mKeys[iter->second];
			}

			const TKeysArray& GetKeys() const { return mKeys; }

			TDE2_API std::vector<F32> GetSamples() const override
			{
				std::vector<F32> samples;
				std::transform(mKeys.cbegin(), mKeys.cend(), std::back_inserter(samples), [](const TKeyFrameType& key) { return key.mTime; });
				return samples;
			}

			TDE2_API TAnimationTrackKeyId GetKeyHandleByTime(F32 value) const override
			{
				/// \note Find the entity in mKeys array
				auto it = std::find_if(mKeys.cbegin(), mKeys.cend(), [value](const TKeyFrameType& key) { return CMathUtils::Abs(key.mTime - value) < 1e-3f; });
				if (it == mKeys.cend())
				{
					return TAnimationTrackKeyId::Invalid;
				}
				
				/// \note Compute an index of the key
				const U32 index = static_cast<U32>(std::distance(mKeys.cbegin(), it));

				/// \note Search for this in the hash table
				auto hashTableIter = std::find_if(mKeysHandlesMap.cbegin(), mKeysHandlesMap.cend(), [index](auto&& entity) { return entity.second == index; });
				if (hashTableIter == mKeysHandlesMap.cend())
				{
					return TAnimationTrackKeyId::Invalid;
				}

				return hashTableIter->first;
			}

			TDE2_API const std::string& GetPropertyBinding() const override { return mPropertyBinding; }
			TDE2_API const std::string& GetName() const override { return mName; }

			TDE2_API E_ANIMATION_INTERPOLATION_MODE_TYPE GetInterpolationMode() const override
			{
				return mInterpolationMode;
			}

			TDE2_API IAnimationClip* GetOwner() override { return mpTrackOwnerAnimation; }

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseAnimationTrack)

			TDE2_API virtual E_RESULT_CODE _saveKeyFrameValue(const TKeyFrameType& value, IArchiveWriter* pWriter) = 0;
			TDE2_API virtual E_RESULT_CODE _loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader) = 0;

			TDE2_API virtual TKeyFrameType _lerpKeyFrames(const TKeyFrameType& left, const TKeyFrameType& right, F32 t) const = 0;
			TDE2_API virtual TKeyFrameType _cubicInterpolation(const TKeyFrameType& left, const TKeyFrameType& right, F32 t, F32 frameDelta) const = 0;

			/*!
				\brief The method returns an index of a key in the array that's time lesser than given
			*/

			TDE2_API I32 _getFrameIndexByTime(F32 time) const
			{
				if (mKeys.size() <= 1)
				{
					return -1;
				}

				F32 t = time;

				const F32 startTime = mKeys.front().mTime;

				if (mpTrackOwnerAnimation->GetWrapMode() == E_ANIMATION_WRAP_MODE_TYPE::LOOP)
				{
					const F32 endTime = mKeys[mKeys.size() - 1].mTime;

					const F32 duration = endTime - startTime;

					t = fmodf(time - startTime, duration);  // clamp time with duration

					if (t < 0.0f)
					{
						t += duration;
					}

					t += startTime;
				}
				else
				{
					if (CMathUtils::IsLessOrEqual(time, startTime))
					{
						return 0;
					}

					if (CMathUtils::IsGreatOrEqual(time, mKeys[mKeys.size() - 2].mTime))
					{
						return static_cast<I32>(mKeys.size()) - 2;
					}
				}

				for (auto it = mKeys.rbegin(); it != mKeys.rend(); ++it)
				{
					if (CMathUtils::IsGreatOrEqual(time, it->mTime))
					{
						return static_cast<I32>(std::distance(it, mKeys.rend()) - 1);
					}
				}

				TDE2_UNREACHABLE();
				return -1;
			}

			TDE2_API F32 _adjustTrackTime(F32 time) const
			{
				if (mKeys.size() <= 1)
				{
					return 0.0f;
				}

				const F32 startTime = mKeys.front().mTime;
				const F32 endTime = mKeys[mKeys.size() - 1].mTime;
				const F32 duration = endTime - startTime;

				if (CMathUtils::IsLessOrEqual(duration, 0.0f))
				{
					return 0.0f;
				}

				if (mpTrackOwnerAnimation->GetWrapMode() == E_ANIMATION_WRAP_MODE_TYPE::LOOP)
				{
					F32 t = fmodf(time - startTime, duration);  // clamp time with duration

					if (t < 0.0f)
					{
						t += duration;
					}

					t += startTime;
				}
				else
				{
					if (CMathUtils::IsLessOrEqual(time, startTime))
					{
						return startTime;
					}

					if (CMathUtils::IsGreatOrEqual(time, mKeys[mKeys.size() - 1].mTime))
					{
						return endTime;
					}
				}

				return time;
			}

			TDE2_API TKeyFrameType _sample(F32 time) const
			{
				switch (mInterpolationMode)
				{
					case E_ANIMATION_INTERPOLATION_MODE_TYPE::CONSTANT:
						return _sampleConstant(time);
					
					case E_ANIMATION_INTERPOLATION_MODE_TYPE::LINEAR:
						return _sampleLinear(time);
					
					case E_ANIMATION_INTERPOLATION_MODE_TYPE::CUBIC:
						return _sampleCubic(time);
				}

				TDE2_UNREACHABLE();
				return TKeyFrameType();
			}

			TDE2_API TKeyFrameType _sampleConstant(F32 time) const
			{
				const I32 index = _getFrameIndexByTime(time);
				if (index < 0 || index >= static_cast<I32>(mKeys.size()))
				{
					return TKeyFrameType();
				}

				return mKeys[index];
			}

			TDE2_API std::tuple<TKeyFrameType, TKeyFrameType, F32, F32> _getInterpolationData(F32 time) const
			{
				static const auto InvalidData = std::make_tuple(TKeyFrameType(), TKeyFrameType(), time, 0.0f);

				const I32 index = _getFrameIndexByTime(time);
				if (index < 0 || index >= static_cast<I32>(mKeys.size()) - 1)
				{
					return InvalidData;
				}

				const I32 nextIndex = index + 1;

				const F32 trackTime = _adjustTrackTime(time);
				const F32 thisTime = mKeys[index].mTime;

				const F32 frameDelta = mKeys[nextIndex].mTime - thisTime;

				if (CMathUtils::IsLessOrEqual(frameDelta, 0.0f))
				{
					return InvalidData;
				}

				const F32 t = (trackTime - thisTime) / frameDelta;

				return { mKeys[index], mKeys[nextIndex], t, frameDelta };
			}

			TDE2_API TKeyFrameType _sampleLinear(F32 time) const
			{
				F32 frameDelta, t;
				TKeyFrameType currKey, nextKey;

				std::tie(currKey, nextKey, t, frameDelta) = _getInterpolationData(time);

				return _lerpKeyFrames(currKey, nextKey, t);
			}

			TDE2_API TKeyFrameType _sampleCubic(F32 time) const
			{
				F32 frameDelta, t;
				TKeyFrameType currKey, nextKey;

				std::tie(currKey, nextKey, t, frameDelta) = _getInterpolationData(time);

				return _cubicInterpolation(currKey, nextKey, t, frameDelta);
			}
		protected:
			static const std::string mNameKeyId;
			static const std::string mBindingKeyId;
			static const std::string mInterpolationModeKeyId;

			IAnimationClip* mpTrackOwnerAnimation;

			std::string mName;
			std::string mPropertyBinding; ///< Format of the bindings: component_name.property_name

			TKeysHandleRegistry mKeysHandlesMap;
			TKeysArray mKeys;

			E_ANIMATION_INTERPOLATION_MODE_TYPE mInterpolationMode;
	};


	template <typename T> CBaseAnimationTrack<T>::CBaseAnimationTrack(): CBaseObject() {}


	template <typename T> const std::string CBaseAnimationTrack<T>::mNameKeyId = "name";
	template <typename T> const std::string CBaseAnimationTrack<T>::mBindingKeyId = "property_binding";
	template <typename T> const std::string CBaseAnimationTrack<T>::mInterpolationModeKeyId = "interpolation_mode";
}