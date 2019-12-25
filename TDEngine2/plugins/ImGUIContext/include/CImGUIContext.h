/*!
	/file CImGUIContext.h
	/date 01.12.2019
	/authors Kasimov Ildar
*/

#pragma once


#include <core/IImGUIContext.h>
#include <math/TVector2.h>
#include <math/TVector4.h>
#include <utils/Color.h>


struct ImGuiIO;
struct ImDrawData;


namespace TDEngine2
{
	class IGraphicsContext;
	class IResourceHandler;
	class IVertexBuffer;
	class IIndexBuffer;


	/*!
		\brief A factory function for creation objects of CImGUIContext's type

		\param[in, out] pWindowSystem A pointer to IWindowSystem implementation
		\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
		\param[in, out] pResourceManager A pointer to IResourceManager implementation
		\param[in, out] pInputContext A pointer to IInputContext implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CImGUIContext's implementation
	*/

	TDE2_API IImGUIContext* CreateImGUIContext(IWindowSystem* pWindowSystem, IGraphicsObjectManager* pGraphicsObjectManager,
											   IResourceManager* pResourceManager, IInputContext* pInputContext, E_RESULT_CODE& result);


	/*!
		class CImGUIContext

		\brief The class is an implementation of an immediate mode GUI for editor's stuffs
	*/

	class CImGUIContext : public IImGUIContext
	{
		public:
			friend TDE2_API IImGUIContext* CreateImGUIContext(IWindowSystem* pWindowSystem, IGraphicsObjectManager* pGraphicsObjectManager,
															  IResourceManager* pResourceManager, IInputContext* pInputContext, E_RESULT_CODE& result);
		protected:
			typedef struct TEditorUIVertex
			{
				TVector4  mPos;
				TColor32F mColor;
				TVector2  mUV;
			} TEditorUIVertex, *TEditorUIVertexPtr;
		public:
			/*!
				\brief The method initializes an internal state of a context

				\param[in, out] pWindowSystem A pointer to IWindowSystem implementation
				\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
				\param[in, out] pResourceManager A pointer to IResourceManager implementation
				\param[in, out] pInputContext A pointer to IInputContext implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IWindowSystem* pWindowSystem, IGraphicsObjectManager* pGraphicsObjectManager,
										IResourceManager* pResourceManager, IInputContext* pInputContext) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method configures the immediate GUI context for WIN32 platform

				\param[in] pWindowSystem A pointer to CWin32WindowSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ConfigureForWin32Platform(const CWin32WindowSystem* pWindowSystem);

			/*!
				\brief The method configures the immediate GUI context for UNIX platform

				\param[in] pWindowSystem A pointer to CUnixWindowSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ConfigureForUnixPlatform(const CUnixWindowSystem* pWindowSystem);
			
			/*!
				\brief The method begins to populate immediage GUI state. Any UI element should be drawn during
				BeginFrame/EndFrame scope

				\param[in] dt Time elapsed from last frame was rendered
			*/

			TDE2_API void BeginFrame(float dt) override;

			/*!
				\brief The method flushes current state and send all the data onto GPU to render it
			*/

			TDE2_API void EndFrame() override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CImGUIContext)

			TDE2_API E_RESULT_CODE _initInternalImGUIContext(ImGuiIO& io);

			TDE2_API void _updateInputState(ImGuiIO& io, IInputContext* pInputContext);

			TDE2_API E_RESULT_CODE _initGraphicsResources(ImGuiIO& io, IGraphicsContext* pGraphicsContext, IGraphicsObjectManager* pGraphicsManager,
														  IResourceManager* pResourceManager);

			TDE2_API E_RESULT_CODE _initSystemFonts(ImGuiIO& io, IResourceManager* pResourceManager, IGraphicsObjectManager* pGraphicsManager);

			TDE2_API void _engineInternalRender(ImDrawData* pImGUIData);
		protected:
			std::atomic_bool        mIsInitialized;

			IWindowSystem*          mpWindowSystem;

			IGraphicsContext*       mpGraphicsContext;

			IGraphicsObjectManager* mpGraphicsObjectManager;

			IResourceManager*       mpResourceManager;

			IInputContext*          mpInputContext;

			ImGuiIO*                mpIOContext;

			// All the stuffs below form a material instance
			IResourceHandler*       mpFontTextureHandler;

			TTextureSamplerId       mFontTextureSamplerHandle;

			TBlendStateId           mBlendStateHandle;

			TDepthStencilStateId    mDepthStencilStateHandle;

			IVertexBuffer*          mpVertexBuffer;

			IIndexBuffer*           mpIndexBuffer;
	};
}