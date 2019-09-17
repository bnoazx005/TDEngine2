/*!
	\file CDebugUtility.h
	\date 17.09.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "IDebugUtility.h"
#include "./../core/CBaseObject.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of IDebugUtility's type.

		\param[in, out] pGraphicsObjectManager A pointer to implementation of IGraphicsObjectManager interface

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IDebugUtility's implementation
	*/

	TDE2_API IDebugUtility* CreateDebugUtility(IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);


	/*!
		class CDebugUtility

		\brief The class implements debug helper to
		draw information on a screen
	*/

	class CDebugUtility : public IDebugUtility, public CBaseObject
	{
		public:
			friend TDE2_API IDebugUtility* CreateDebugUtility(IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an initial state of the object

				\param[in, out] pGraphicsObjectManager A pointer to implementation of IGraphicsObjectManager interface

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsObjectManager* pGraphicsObjectManager) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method adds a request to draw a line with a given parameters to rendering queue

				\param[in] start A start position of a line's segment
				\param[in] end An end position of a line's segment
				\param[in] color A line's color
			*/

			TDE2_API void DrawLine(const TVector3& start, const TVector3& end, const TColor32F& color) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CDebugUtility)
		protected:
			IGraphicsObjectManager* mpGraphicsObjectManager;
	};
}