/*!
	\file ISelectionManager.h
	\date 27.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Types.h"
#include "../core/IBaseObject.h"
#include "../core/Event.h"
#include "../math/TVector2.h"
#include <functional>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class IEditorsManager;
	class IWorld;
	class IResourceManager;
	class IWindowSystem;
	class IGraphicsContext;


	enum class TTextureHandleId : U32;


	TDE2_DECLARE_SCOPED_PTR(IResourceManager)
	TDE2_DECLARE_SCOPED_PTR(IWindowSystem)
	TDE2_DECLARE_SCOPED_PTR(IGraphicsContext)
	TDE2_DECLARE_SCOPED_PTR(IWorld)


	/*!
		interface ISelectionManager

		\brief The interface describes a functionality of a manager that stores information
		about selected objects
	*/
	
	class ISelectionManager: public virtual IBaseObject, public IEventHandler
	{
		public:
			/*!
				\brief The method initializes the internal state of the manager

				\param[in, out] pResourceManager A pointer to IResourceManager implementation
				\param[in, out] pWindowSystem A pointer to IWindowSystem implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation
				\param[in, out] pEditorsManager A pointer to IEditorsManager implementation. Don't store CScopedPtr pointer to prevent circular dependencies

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TPtr<IResourceManager> pResourceManager, TPtr<IWindowSystem> pWindowSystem, TPtr<IGraphicsContext> pGraphicsContext, IEditorsManager* pEditorsManager) = 0;

			/*!
				\brief The method is used to update internal selection buffer based on incoming texture with selectables data

				\param[in] newSelectionMapHandle A handle that point to ITexture2D instance that hold all the on-screen objects that can be selected

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UpdateSelectionsBuffer(TTextureHandleId newSelectionMapHandle) = 0;

			/*!
				\brief The method tries to pick an object 

				\param[in] position A position of a cursor in which the method tries to pick some object

				\return The method returns an identifier of an entity
			*/

			TDE2_API virtual TEntityId PickObject(const TVector2& position) = 0;

			/*!
				\brief The method allows to manually assign current selected entity by its identifier

				\param[in] id An identifier of entity that should be selected

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetSelectedEntity(TEntityId id) = 0;

			/*!
				\brief The method adds another entity into the selection list. Therefore allows to make
				multiselection rather than SetSelectedEntity that reset current list and selects only the entity

				\param[in] id An identifier of entity that should be selected

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE AddSelectedEntity(TEntityId id) = 0;

			TDE2_API virtual E_RESULT_CODE ClearSelection() = 0;

			/*!
				\brief The method sets up a pointer to IWorld instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetWorldInstance(TPtr<IWorld> pWorld) = 0;

			/*!
				\brief The method returns an identifier of an entity

				\return The method returns an identifier of an entity
			*/

			TDE2_API virtual TEntityId GetSelectedEntityId() const = 0;

			TDE2_API virtual const std::vector<TEntityId>& GetSelectedEntities() const = 0;

			/*!
				\return The method returns true when the given entity identifier was marked as selected sooner and false in other cases
			*/

			TDE2_API virtual bool IsEntityBeingSelected(TEntityId id) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISelectionManager)
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(ISelectionManager)


	/*!
		struct TOnObjectSelected

		\brief The s describes a functionality of an event
	*/

	typedef struct TOnObjectSelected: TBaseEvent
	{
		TDE2_EVENT(TOnObjectSelected);
		
		IWorld*   mpWorld = nullptr;

		TEntityId mObjectID = TEntityId::Invalid;
	} TOnObjectSelected, *TBaseEventTOnObjectSelectedPtr;
}

#endif