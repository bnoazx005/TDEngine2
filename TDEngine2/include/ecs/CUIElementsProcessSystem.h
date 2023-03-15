/*!
	\file CUIElementsProcessSystem.h
	\date 28.04.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include "IWorld.h"
#include "../graphics/UI/CToggleComponent.h"
#include "../graphics/UI/CInputReceiverComponent.h"
#include <vector>


namespace TDEngine2
{
	class IGraphicsContext;
	class IResourceManager;
	class CUIElementMeshData;
	class CLayoutElement;
	class CTransform;
	class CImage;
	class C9SliceImage;
	class CLabel;
	class CCanvas;
	class CGridGroupLayout;


	/*!
		\brief A factory function for creation objects of CUIElementsProcessSystem's type.

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[in, out] pResourceManager A pointer to IResourceManager implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CUIElementsProcessSystem's implementation
	*/

	TDE2_API ISystem* CreateUIElementsProcessSystem(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, E_RESULT_CODE& result);


	/*!
		class CUIElementsProcessSystem

		\brief The class is a system that processes UI elements such as canvases and LayoutElement components
	*/

	class CUIElementsProcessSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateUIElementsProcessSystem(IGraphicsContext*, IResourceManager*, E_RESULT_CODE&);
		public:
			template <typename TComponentType>
			struct TUIRenderableElementsContext
			{
				std::vector<TComponentType*>     mpRenderables;
				std::vector<CLayoutElement*>     mpLayoutElements;
				std::vector<CUIElementMeshData*> mpUIMeshes;
				std::vector<CTransform*>         mpTransforms;
			};

			struct TCanvasesContext
			{
				std::vector<CCanvas*>        mpCanvases;
				std::vector<CLayoutElement*> mpLayoutElements;
				std::vector<CTransform*>     mpTransforms;
			};

			struct TGridGroupsContext
			{
				std::vector<CGridGroupLayout*> mpGridGroupLayouts;
				std::vector<CLayoutElement*>   mpLayoutElements;
				std::vector<CTransform*>       mpTransforms;
			};

			struct TLayoutElementsContext
			{
				std::vector<CLayoutElement*> mpLayoutElements;
				std::vector<CTransform*>     mpTransforms;
				std::vector<USIZE>           mChildToParentTable;
				std::vector<TEntityId>       mEntities;
			};

			typedef TComponentsQueryLocalSlice<CToggle, CInputReceiver> TTogglesContext;
		public:
			TDE2_SYSTEM(CUIElementsProcessSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in, out] pResourceManager A pointer to IResourceManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager);

			/*!
				\brief The method inject components array into a system

				\param[in] pWorld A pointer to a main scene's object
			*/

			TDE2_API void InjectBindings(IWorld* pWorld) override;

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object

				\param[in] dt A delta time's value
			*/

			TDE2_API void Update(IWorld* pWorld, F32 dt) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUIElementsProcessSystem)
		protected:
			TLayoutElementsContext                     mLayoutElementsContext;
			TCanvasesContext                           mCanvasesContext;
			TGridGroupsContext                         mGridGroupLayoutsContext;
			TTogglesContext                            mTogglesContext;

			TUIRenderableElementsContext<CImage>       mImagesContext;
			TUIRenderableElementsContext<C9SliceImage> mSlicedImagesContext;
			TUIRenderableElementsContext<CLabel>       mLabelsContext;

			IGraphicsContext*                          mpGraphicsContext;

			IResourceManager*                          mpResourceManager;
	};
}