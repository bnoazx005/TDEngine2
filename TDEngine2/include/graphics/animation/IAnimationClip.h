/*!
	\file IAnimationClip.h
	\date 15.04.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Config.h"
#include "../../utils/Utils.h"
#include "../../core/IResourceFactory.h"
#include "../../core/IResourceLoader.h"
#include "../../core/Serialization.h"


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;

	
	/*!
		enum class E_ANIMATION_WRAP_MODE_TYPE

		\brief The enumeration defines allowed types of animation playback
	*/

	enum class E_ANIMATION_WRAP_MODE_TYPE: U8
	{
		PLAY_ONCE,
		LOOP,
	};


	/*!
		struct TAnimationClipParameters

		\brief The stucture contains minimal information for creation IAnimationClip objects
	*/

	typedef struct TAnimationClipParameters : TBaseResourceParameters
	{
		F32                        mDuration = 1.0f;

		E_ANIMATION_WRAP_MODE_TYPE mWrapMode = E_ANIMATION_WRAP_MODE_TYPE::PLAY_ONCE; ///< The wrap mode could be overwritten by animation component
	} TAnimationClipParameters, *TAnimationClipParametersPtr;


	/*!
		interface IAnimationClip

		\brief The interface describes API of animation clips resources
	*/

	class IAnimationClip: public ISerializable
	{
		public:
			/*!
				\brief The method initializes a state of a brand new animation clip.

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] name A resource's name
				\param[in] params Additional parameters of a clip

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												const TAnimationClipParameters& params) = 0;
			
			/*!
				\brief The method initializes an internal state of an animation clip

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) = 0;

			/*!
				\brief The method specifies duration of the clip

				\param[in] duration A time length of the animation clip, should be positive value

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetDuration(F32 duration) = 0;

			/*!
				\brief The method specified wrapping mode for the clip. Remember that this value probably would
				be overwritten when the clip would be assigned into animation component

				\param[in] mode The value determines how the clip would be played
			*/

			TDE2_API virtual void SetWrapMode(E_ANIMATION_WRAP_MODE_TYPE mode) = 0;

			/*!
				\brief The method returns duration of the animation clip
				\return The method returns duration of the animation clip
			*/

			TDE2_API virtual F32 GetDuration() const = 0;

			/*!
				\brief The method returns wrap mode that's defined for the clip
				\return The method returns wrap mode that's defined for the clip
			*/

			TDE2_API virtual E_ANIMATION_WRAP_MODE_TYPE GetWrapMode() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IAnimationClip)
	};


	/*!
		interface IAnimationClipLoader

		\brief The interface describes a functionality of a loader of animation clips
	*/

	class IAnimationClipLoader : public IResourceLoader
	{
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IAnimationClipLoader)
	};


	/*!
		interface IAnimationClipFactory

		\brief The interface describes a functionality of an animation clips factory
	*/

	class IAnimationClipFactory : public IResourceFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IAnimationClipFactory)
	};
}