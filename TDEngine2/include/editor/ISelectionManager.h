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


	/*!
		interface ISelectionManager

		\brief The interface describes a functionality of a manager that stores information
		about selected objects
	*/
	
	class ISelectionManager: public virtual IBaseObject, public IEventHandler
	{
		public:
			typedef std::function<void()> TRenderFrameCallback;
		public:
			/*!
				\brief The method initializes the internal state of the manager

				\param[in, out] pResourceManager A pointer to IResourceManager implementation
				\param[in, out] pWindowSystem A pointer to IWindowSystem implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation
				\param[in, out] pEditorsManager A pointer to IEditorsManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IWindowSystem* pWindowSystem, IGraphicsContext* pGraphicsContext, IEditorsManager* pEditorsManager) = 0;

			/*!
				\brief The method builds so called selection map which is a render target that contains
				information about which object occupies each pixel of the screen

				\param[in] onDrawVisibleObjectsCallback A callback in which all visible objects should be drawn

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE BuildSelectionMap(const TRenderFrameCallback& onDrawVisibleObjectsCallback) = 0;

			/*!
				\brief The method tries to pick an object 

				\param[in] position A position of a cursor in which the method tries to pick some object

				\return The method returns an identifier of an entity
			*/

			TDE2_API virtual TEntityId PickObject(const TVector2& position) = 0;

			/*!
				\brief The method sets up a pointer to IWorld instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetWorldInstance(IWorld* pWorld) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISelectionManager)
	};
}

#endif