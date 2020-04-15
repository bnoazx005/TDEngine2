/*!
	\file IAnimationClip.h
	\date 15.04.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Utils.h"
#include "../core/IResourceFactory.h"
#include "../core/IResourceLoader.h"
#include "../core/Serialization.h"


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;


	/*!
		struct TAnimationClipParameters

		\brief The stucture contains minimal information for creation IAnimationClip objects
	*/

	typedef struct TAnimationClipParameters : TBaseResourceParameters
	{
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