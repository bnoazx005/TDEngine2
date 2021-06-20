/*!
	\file CSkeleton.h
	\date 16.06.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "ISkeleton.h"
#include "../core/CBaseResource.h"
#include "../graphics/IIndexBuffer.h"
#include <vector>
#include <unordered_map>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CSkeleton's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation
		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[in] name A resource's name
		\param[in] id An identifier of a resource
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSkeleton's implementation
	*/

	TDE2_API ISkeleton* CreateSkeleton(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);


	/*!
		class CSkeleton

		\brief The class describes a functionality of a skeleton for skinned meshes
	*/

	class CSkeleton: public ISkeleton, public CBaseResource
	{
		public:
			friend TDE2_API ISkeleton* CreateSkeleton(IResourceManager*, IGraphicsContext*, const std::string&, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_RESOURCE_TYPE(CSkeleton)
			TDE2_REGISTER_TYPE(CSkeleton)

			/*!
				\brief The method initializes an internal state of a mesh object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) override;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			TDE2_API TResult<U32> CreateJoint(const std::string& name, const TMatrix4& bindTransform = IdentityMatrix4, I32 parent = -1) override;

			TDE2_API E_RESULT_CODE RemoveJoint(const std::string& name) override;
			TDE2_API E_RESULT_CODE RemoveJoint(U32 id) override;

			/*!
				\param The method returns a joint with specified identifier, pass 0 to get a root joint
				\return A pointer to TJoint or an error code if there is no a joint with the given identifier
			*/

			TDE2_API TResult<TJoint*> GetJoint(U32 id) override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSkeleton)

			TDE2_API E_RESULT_CODE _postLoad();

			TDE2_API const IResourceLoader* _getResourceLoader() override;
		protected:
			std::vector<TJoint> mJoints;
	};



	/*!
		\brief A factory function for creation objects of CSkeletonLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[in, out] pFileSystem A pointer to IFileSystem's implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSkeletonLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateSkeletonLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
													 E_RESULT_CODE& result);


	/*!
		class CSkeletonLoader

		\brief The class implements a functionality of a base material loader
	*/

	class CSkeletonLoader : public CBaseObject, public ISkeletonLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateSkeletonLoader(IResourceManager*, IGraphicsContext*, IFileSystem*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method loads data into the specified resource based on its
				internal information

				\param[in, out] pResource A pointer to an allocated resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadResource(IResource* pResource) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the loader serves

				\return The method returns an identifier of a resource's type, which
				the loader serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSkeletonLoader)
		protected:
			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;

			IGraphicsContext* mpGraphicsContext;
	};


	/*!
		\brief A factory function for creation objects of CSkeletonFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSkeletonFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateSkeletonFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CSkeletonFactory

		\brief The class is an abstract factory of CSkeleton objects that
		is used by a resource manager
	*/

	class CSkeletonFactory : public CBaseObject, public ISkeletonFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateSkeletonFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a material factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* Create(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSkeletonFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}