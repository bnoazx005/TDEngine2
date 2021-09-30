/*!
	\file EditorComponents.h
	\date 09.03.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../ecs/CBaseComponent.h"
#include <vector>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	TDE2_DECLARE_FLAG_COMPONENT(SelectedEntityComponent)


	enum class TSceneId: U32;

	/*!
		\brief A factory function for creation objects of CSceneInfoComponent's type.

		\param[in] sceneId An identifier of a scene
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IComponent's implementation
	*/

	TDE2_API IComponent* CreateSceneInfoComponent(const std::string& sceneId, E_RESULT_CODE& result);


	class CSceneInfoComponent : public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateSceneInfoComponent(const std::string&, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CSceneInfoComponent)

			/*!
				\brief The method initializes an internal state of a transform

				\param[in] sceneId An identifier of a scene
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(const std::string& sceneId);

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


	/*!
		\brief A factory function for creation objects of CSceneInfoComponentFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSceneInfoComponentFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateSceneInfoComponentFactory(E_RESULT_CODE& result);


	/*!
		class CSceneInfoComponentFactory

		\brief The class is factory facility to create a new objects of CSceneInfoComponent type
	*/

	class CSceneInfoComponentFactory : public IComponentFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateSceneInfoComponentFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init();

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* Create(const TBaseComponentParameters* pParams) const override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* CreateDefault(const TBaseComponentParameters& params) const override;

			/*!
				\brief The method returns an identifier of a component's type, which
				the factory serves

				\return The method returns an identifier of a component's type, which
				the factory serves
			*/

			TDE2_API TypeId GetComponentTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSceneInfoComponentFactory)
	};
}

#endif