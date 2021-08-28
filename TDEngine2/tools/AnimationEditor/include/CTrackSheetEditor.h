/*!
	\file CTrackSheetEditor.h
	\date 27.08.2021
	\authors Kasimov Ildar
*/

#pragma once


#include <TDEngine2.h>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CTrackSheetEditor's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API class CTrackSheetEditor* CreateTrackSheetEditor(E_RESULT_CODE& result);


	/*!
		class CTrackSheetEditor

		\brief The class is an implementation of a widget for editing animation tracks
	*/

	class CTrackSheetEditor : public CBaseObject
	{
	public:
		friend TDE2_API CTrackSheetEditor* CreateTrackSheetEditor(E_RESULT_CODE&);

	public:
		/*!
			\brief The method initializes internal state of the editor

			\param[in, out] pResourceManager A pointer to implementation of IResourceManager
			\param[in, out] pWorld A pointer to implementation of IWorld

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Init();

		/*!
			\brief The method frees all memory occupied by the object

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Free() override;
	protected:
		DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CTrackSheetEditor)

	protected:
	};
}