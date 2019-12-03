/*!
	/file CImGUIContext.h
	/date 01.12.2019
	/authors Kasimov Ildar
*/

#pragma once


#include <core/IImGUIContext.h>


namespace TDEngine2
{
	class IGraphicsContext;


	/*!
		\brief A factory function for creation objects of CImGUIContext's type

		\param[in, out] pWindowSystem A pointer to IWindowSystem implementation
		\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
		\param[in, out] pInputContext A pointer to IInputContext implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CImGUIContext's implementation
	*/

	TDE2_API IImGUIContext* CreateImGUIContext(IWindowSystem* pWindowSystem, IGraphicsObjectManager* pGraphicsObjectManager,
											   IInputContext* pInputContext, E_RESULT_CODE& result);


	/*!
		class CImGUIContext

		\brief The class is an implementation of an immediate mode GUI for editor's stuffs
	*/

	class CImGUIContext : public IImGUIContext
	{
		public:
			friend TDE2_API IImGUIContext* CreateImGUIContext(IWindowSystem* pWindowSystem, IGraphicsObjectManager* pGraphicsObjectManager,
															  IInputContext* pInputContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a context

				\param[in, out] pWindowSystem A pointer to IWindowSystem implementation
				\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
				\param[in, out] pInputContext A pointer to IInputContext implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IWindowSystem* pWindowSystem, IGraphicsObjectManager* pGraphicsObjectManager,
										IInputContext* pInputContext) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;
			
			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CImGUIContext)
		protected:
			std::atomic_bool        mIsInitialized;

			IWindowSystem*          mpWindowSystem;

			IGraphicsContext*       mpGraphicsContext;

			IGraphicsObjectManager* mpGraphicsObjectManager;

			IInputContext*          mpInputContext;
	};
}