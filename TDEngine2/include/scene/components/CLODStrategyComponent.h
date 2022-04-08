/*!
	\file CLODStrategyComponent.h
	\date 07.04.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include <vector>


namespace TDEngine2
{
	typedef struct TLODStrategyComponentParameters : TBaseComponentParameters
	{
	} TLODStrategyComponentParameters, *TLODStrategyComponentParametersPtr;


	enum class E_LOD_INSTANCE_ACTIVE_PARAMS : U8
	{
		NONE = 0,
		MESH_ID = 1 << 1,
		SUBMESH_ID = 1 << 2,
		MATERIAL_ID = 1 << 3,
	};

	TDE2_DECLARE_BITMASK_OPERATORS_INTERNAL(E_LOD_INSTANCE_ACTIVE_PARAMS);


	struct TLODInstanceInfo
	{
		F32                          mSwitchDistance = 1000.0f;

		std::string                  mMeshId;
		std::string                  mSubMeshId;
		std::string                  mMaterialId;

		E_LOD_INSTANCE_ACTIVE_PARAMS mActiveParams = E_LOD_INSTANCE_ACTIVE_PARAMS::NONE;
	};


	/*!
		\brief A factory function for creation objects of CLODStrategyComponent's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CLODStrategyComponent's implementation
	*/

	TDE2_API IComponent* CreateLODStrategyComponent(E_RESULT_CODE& result);


	/*!
		class CLODStrategyComponent

		\brief The class represents omni directional light source 
	*/

	class CLODStrategyComponent : public CBaseComponent, public CPoolMemoryAllocPolicy<CLODStrategyComponent, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateLODStrategyComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CLODStrategyComponent)

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

			TDE2_API E_RESULT_CODE AddLODInstance(const TLODInstanceInfo& info);

			TDE2_API E_RESULT_CODE RemoveLODInstance(U32 index);

			TDE2_API void Sort();

			TDE2_API void ForEachInstance(const std::function<bool(USIZE, TLODInstanceInfo&)>& action);

			TDE2_API TLODInstanceInfo* GetLODInfo(U32 index);
			TDE2_API TLODInstanceInfo* GetLODInfo(F32 distanceToCamera);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLODStrategyComponent)
		protected:
			std::vector<TLODInstanceInfo> mLODInstances;
			bool                          mIsDirty = true;
	};


	TDE2_DECLARE_COMPONENT_FACTORY(LODStrategyComponent, TLODStrategyComponentParameters);
}
