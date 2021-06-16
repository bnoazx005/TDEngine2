/*!
	\file ISkeleton.h
	\date 16.06.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/IResource.h"
#include "../core/IResourceFactory.h"
#include "../core/IResourceLoader.h"
#include "../core/Serialization.h"


namespace TDEngine2
{
	/*!
		struct TSkeleton2DParameters

		\brief The stucture contains fields for creation ISkeleton and its derived classes
	*/

	typedef struct TSkeletonParameters : TBaseResourceParameters
	{
	} TSkeletonParameters, *TSkeletonParametersPtr;


	/*!
		interface ISkeleton

		\brief The interface describes a functionality of a skeleton for skinned meshes
	*/

	class ISkeleton: public ISerializable
	{
		public:
			TDE2_REGISTER_TYPE(ISkeleton);

			/*!
				\brief The method initializes an internal state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) = 0;

		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISkeleton)
	};


	class ISkeletonLoader : public IGenericResourceLoader<IResourceManager*, IGraphicsContext*, IFileSystem*> {};
	class ISkeletonFactory : public IGenericResourceFactory<IResourceManager*, IGraphicsContext*> {};
}