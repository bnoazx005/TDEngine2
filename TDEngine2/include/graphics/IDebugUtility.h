/*!
	\file IDebugUtility.h
	\date 17.09.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Config.h"
#include "./../utils/Types.h"
#include "./../core/IBaseObject.h"
#include "./../math/TVector3.h"
#include "./../utils/Color.h"


namespace TDEngine2
{
	class IGraphicsObjectManager;


	/*!
		interface IDebugUtility

		\brief The interfaces describes a functionality of a debug helper,
		which provides methods to draw gizmos, lines, text, to simplify
		debugging process
	*/

	class IDebugUtility: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an initial state of the object

				\param[in, out] pGraphicsObjectManager A pointer to implementation of IGraphicsObjectManager interface

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IGraphicsObjectManager* pGraphicsObjectManager) = 0;

			/*!
				\brief The method adds a request to draw a line with a given parameters to rendering queue

				\param[in] start A start position of a line's segment
				\param[in] end An end position of a line's segment
				\param[in] color A line's color
			*/

			TDE2_API virtual void DrawLine(const TVector3& start, const TVector3& end, const TColor32F& color) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IDebugUtility)
	};
}