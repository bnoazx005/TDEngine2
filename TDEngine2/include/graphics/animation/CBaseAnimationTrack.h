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

				mIsInitialized = true;

				return RC_OK;
			}

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override
			{
				if (!mIsInitialized)
				{
					return RC_FAIL;
				}

				--mRefCounter;

				if (!mRefCounter)
				{
					mIsInitialized = false;
					delete this;
				}

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

				pWriter->BeginGroup("keys", true);

				for (const TKeyFrameType& currKey : mKeys)
				{
					pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
					{
						pWriter->BeginGroup("key");

						pWriter->SetFloat("time", currKey.mTime);

						pWriter->BeginGroup("value");
						_saveKeyFrameValue(currKey, pWriter);
						pWriter->EndGroup();

						pWriter->EndGroup();
					}
					pWriter->EndGroup();
				}

				pWriter->EndGroup();

				return RC_OK;
			}

			TDE2_API TAnimationTrackKeyId CreateKey(F32 time) override
			{
				auto it = std::find_if(mKeys.cbegin(), mKeys.cend(), [time](const TKeyFrameType& key) { return CMathUtils::Abs(key.mTime - time) < 1e-3f; });
				if (it != mKeys.cend()) 
				{
					return static_cast<TAnimationTrackKeyId>(std::distance(mKeys.cbegin(), it));
				}

				TAnimationTrackKeyId handle = static_cast<TAnimationTrackKeyId>(mKeys.size());

				mKeys.push_back({ time, {} });

				return handle;
			}

			TDE2_API E_RESULT_CODE RemoveKey(TAnimationTrackKeyId handle) override
			{
				if (handle == TAnimationTrackKeyId::Invalid || static_cast<size_t>(handle) >= mKeys.size())
				{
					return RC_INVALID_ARGS;
				}

				mKeys.erase(mKeys.cbegin() + static_cast<size_t>(handle));

				return RC_OK;
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

			TKeyFrameType* GetKey(TAnimationTrackKeyId handle) const
			{
				if (handle == TAnimationTrackKeyId::Invalid || static_cast<size_t>(handle) >= mKeys.size())
				{
					return nullptr;
				}

				return mKeys[static_cast<U32>(handle)];
			}

			TDE2_API const std::string& GetPropertyBinding() const override { return mPropertyBinding; }
			TDE2_API const std::string& GetName() const override { return mName; }

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseAnimationTrack)

			TDE2_API virtual E_RESULT_CODE _saveKeyFrameValue(const TKeyFrameType& value, IArchiveWriter* pWriter) = 0;
		protected:
			static const std::string mNameKeyId;
			static const std::string mBindingKeyId;

			IAnimationClip* mpTrackOwnerAnimation;

			std::string mName;
			std::string mPropertyBinding; ///< Format of the bindings: component_name.property_name

			TKeysArray mKeys;
	};


	template <typename T> CBaseAnimationTrack<T>::CBaseAnimationTrack(): CBaseObject() {}


	template <typename T> const std::string CBaseAnimationTrack<T>::mNameKeyId = "name";
	template <typename T> const std::string CBaseAnimationTrack<T>::mBindingKeyId = "property_binding";
}