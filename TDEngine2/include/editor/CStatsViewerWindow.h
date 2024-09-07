/*!
	\file CStatsViewerWindow.h
	\date 15.10.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "IEditorWindow.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class IGraphicsContext;


	TDE2_DECLARE_SCOPED_PTR(IGraphicsContext);


	/*!
		\brief A factory function for creation objects of CStatsViewerWindow's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateStatsViewerWindow(TPtr<IGraphicsContext> pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CStatsViewerWindow

		\brief The class is an implementation of an overlayed window with in-game statistics
	*/

	class CStatsViewerWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateStatsViewerWindow(TPtr<IGraphicsContext>, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes internal state of the editor

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TPtr<IGraphicsContext> pGraphicsContext);

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CStatsViewerWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API void _onDraw() override;
		protected:
			TPtr<IGraphicsContext> mpGraphicsContext;
	};
}

#endif