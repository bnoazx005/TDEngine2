/*!
	\file EditorComponents.h
	\date 09.03.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../core/memory/CPoolAllocator.h"
#include "../../ecs/CBaseComponent.h"
#include <vector>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	TDE2_DECLARE_FLAG_COMPONENT(SelectedEntityComponent)


	enum class TSceneId: U32;

	/*!
		\brief A factory function for creation objects of CSceneInfoComponent's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IComponent's implementation
	*/

	TDE2_API IComponent* CreateSceneInfoComponent(E_RESULT_CODE& result);


	class CSceneInfoComponent : public CBaseComponent, public CPoolMemoryAllocPolicy<CSceneInfoComponent, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateSceneInfoComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CSceneInfoComponent)

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

			TDE2_API void SetSceneId(const std::string& id);

			TDE2_API const std::string& GetSceneId() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSceneInfoComponent)
		protected:
			std::string mSceneId;
	};

	
	typedef struct TSceneInfoComponentParameters : public TBaseComponentParameters
	{
		std::string mSceneId;
	} TSceneInfoComponentParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(SceneInfoComponent, TSceneInfoComponentParameters);
}

#endif