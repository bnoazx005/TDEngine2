/*!
	\file ISelectionManager.h
	\date 27.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Types.h"
#include "../core/IBaseObject.h"
#include <functional>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	/*!
		interface ISelectionManager

		\brief The interface describes a functionality of a manager that stores information
		about selected objects
	*/
	
	class ISelectionManager: public virtual IBaseObject
	{
		public:
			typedef std::function<void()> TRenderFrameCallback;
		public:
			/*!
				\brief The method initializes the internal state of the manager

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method builds so called selection map which is a render target that contains
				information about which object occupies each pixel of the screen

				\param[in] onDrawVisibleObjectsCallback A callback in which all visible objects should be drawn

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE BuildSelectionMap(const TRenderFrameCallback& onDrawVisibleObjectsCallback) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISelectionManager)
	};
}

#endif