/*!
	\file CSelectionManager.h
	\date 27.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "ISelectionManager.h"
#include "../core/CBaseObject.h"
#include "../core/Event.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class IEventManager;


	/*!
		\brief A factory function for creation objects of CSelectionManager's type.

		\param[in, out] pResourceManager A pointer to IResourceManager implementation
		\param[in, out] pWindowSystem A pointer to IWindowSystem implementation
		\param[in, out] pEditorsManager A pointer to IEditorsManager implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSelectionManager's implementation
	*/

	TDE2_API ISelectionManager* CreateSelectionManager(TPtr<IResourceManager> pResourceManager, TPtr<IWindowSystem> pWindowSystem, TPtr<IGraphicsContext> pGraphicsContext, 
													   IEditorsManager* pEditorsManager, E_RESULT_CODE& result);


	/*!
		class CSelectionManager

		\brief The class implements a logic of manager that controls all selected objects
	*/

	class CSelectionManager : public CBaseObject, public ISelectionManager
	{
		public:
			friend TDE2_API ISelectionManager* CreateSelectionManager(TPtr<IResourceManager>, TPtr<IWindowSystem>, TPtr<IGraphicsContext>, IEditorsManager*, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CSelectionManager)

			/*!
				\brief The method initializes the internal state of the manager

				\param[in, out] pResourceManager A pointer to IResourceManager implementation
				\param[in, out] pWindowSystem A pointer to IWindowSystem implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation
				\param[in, out] pEditorsManager A pointer to IEditorsManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TPtr<IResourceManager> pResourceManager, TPtr<IWindowSystem> pWindowSystem, TPtr<IGraphicsContext> pGraphicsContext, IEditorsManager* pEditorsManager) override;
			
			/*!
				\brief The method is used to update internal selection buffer based on incoming texture with selectables data

				\param[in] newSelectionMapHandle A handle that point to ITexture2D instance that hold all the on-screen objects that can be selected

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UpdateSelectionsBuffer(TTextureHandleId newSelectionMapHandle) override;

			/*!
				\brief The method receives a given event and processes it

				\param[in] pEvent A pointer to event data

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE OnEvent(const TBaseEvent* pEvent) override;

			/*!
				\brief The method tries to pick an object

				\param[in] position A position of a cursor in which the method tries to pick some object

				\return The method returns an identifier of an entity
			*/

			TDE2_API TEntityId PickObject(const TVector2& position) override;

			/*!
				\brief The method allows to manually assign current selected entity by its identifier

				\param[in] id An identifier of entity that should be selected

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetSelectedEntity(TEntityId id) override;

			/*!
				\brief The method adds another entity into the selection list. Therefore allows to make
				multiselection rather than SetSelectedEntity that reset current list and selects only the entity

				\param[in] id An identifier of entity that should be selected

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE AddSelectedEntity(TEntityId id) override;

			TDE2_API E_RESULT_CODE ClearSelection() override;

			/*!
				\brief The method sets up a pointer to IWorld instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetWorldInstance(TPtr<IWorld> pWorld) override;

			/*!
				\brief The method returns an identifier of a listener

				\return The method returns an identifier of a listener
			*/

			TDE2_API TEventListenerId GetListenerId() const override;

			/*!
				\brief The method returns an identifier of an entity

				\return The method returns an identifier of an entity
			*/

			TDE2_API TEntityId GetSelectedEntityId() const override;

			TDE2_API const std::vector<TEntityId>& GetSelectedEntities() const;

			/*!
				\return The method returns true when the given entity identifier was marked as selected sooner and false in other cases
			*/

			TDE2_API bool IsEntityBeingSelected(TEntityId id) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSelectionManager)

			TDE2_API E_RESULT_CODE _createRenderTarget(U32 width, U32 height);

			TDE2_API E_RESULT_CODE _setSelection(TEntityId id, bool resetSelection = true);
			TDE2_API void _resetCurrentSelection();

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			TPtr<IResourceManager> mpResourceManager;

			IEditorsManager*       mpEditorsManager;

			TPtr<IWindowSystem>    mpWindowSystem;

			TPtr<IGraphicsContext> mpGraphicsContext;

			TPtr<IEventManager>    mpEventManager;

			TPtr<IWorld>           mpWorld;

			TSystemId              mObjectSelectionSystemId;

			TResourceId            mSelectionGeometryBufferHandle; ///< This is a render target that contains all visible geometry 
			TResourceId            mReadableSelectionBufferHandle;

			U32                    mWindowHeaderHeight;

			std::vector<TEntityId> mSelectedEntities;
	};
}

#endif