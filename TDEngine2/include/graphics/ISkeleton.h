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
#include "../math/TMatrix4.h"


namespace TDEngine2
{
	typedef struct TJoint
	{
		U32         mIndex = 0;
		I32         mParentIndex = -1;
		std::string mName;
		TMatrix4    mLocalBindTransform; ///< \note This transformation is a local for specific joint relative to its parent
		TMatrix4    mInvBindTransform; /// \note The field is recomputed on loading of a skeleton, moves from joint space to model space
	} TJoint, *TJointPtr, TBone, *TBonePtr;


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
			static constexpr U32 mMaxNumOfJoints = 256;
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

			TDE2_API virtual TResult<U32> CreateJoint(const std::string& name, const TMatrix4& bindTransform = IdentityMatrix4, I32 parent = -1) = 0;
			
			TDE2_API virtual E_RESULT_CODE RemoveJoint(const std::string& name) = 0;
			TDE2_API virtual E_RESULT_CODE RemoveJoint(U32 id) = 0;

			TDE2_API virtual void ForEachJoint(const std::function<void(TJoint*)>& action = nullptr) = 0;
			TDE2_API virtual void ForEachChildJoint(U32 jointIndex, const std::function<void(TJoint*)>& action = nullptr) = 0;

			/*!
				\param The method returns a joint with specified identifier, pass 0 to get a root joint
				\return A pointer to TJoint or an error code if there is no a joint with the given identifier
			*/

			TDE2_API virtual TResult<TJoint*> GetJoint(U32 id) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISkeleton)
	};


	class ISkeletonLoader : public IGenericResourceLoader<IResourceManager*, IGraphicsContext*, IFileSystem*> {};
	class ISkeletonFactory : public IGenericResourceFactory<IResourceManager*, IGraphicsContext*> {};
}