#pragma once


#include <TDEngine2.h>


struct TVertex
{
	TDEngine2::TVector4 mPos;

	TDEngine2::TVector4 mColor;
};


class CCustomEngineListener: public TDEngine2::IEngineListener
{
	public:
		CCustomEngineListener() = default;
		virtual ~CCustomEngineListener() = default;

		/*!
		\brief The event occurs after the engine was initialized

		\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDEngine2::E_RESULT_CODE OnStart() override;

		/*!
		\brief The event occurs every frame

		\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDEngine2::E_RESULT_CODE OnUpdate(const float& dt) override;

		/*!
		\brief The event occurs before the engine will be destroyed

		\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDEngine2::E_RESULT_CODE OnFree() override;

		/*!
			\brief The method is used to inject a pointer to the engine into the object of this type

			\param[in] An instance of the engine's core
		*/

		void SetEngineInstance(TDEngine2::IEngineCore* pEngineCore) override;
	protected:
		TDEngine2::E_RESULT_CODE _mountResourcesDirectories();

	protected:
		TDEngine2::IEngineCore*             mpEngineCoreInstance;

		TDEngine2::IGraphicsContext*        mpGraphicsContext;

		TDEngine2::IWindowSystem*           mpWindowSystem;

		TDEngine2::IResourceManager*        mpResourceManager;

		TDEngine2::IFileSystem*             mpFileSystem;

		TDEngine2::IGraphicsObjectManager*  mpGraphicsObjectManager;

		TDEngine2::IResourceHandler*        mpShader;
		
		TDEngine2::IVertexBuffer*           mpVertexBuffer;

		TDEngine2::IIndexBuffer*            mpIndexBuffer;

		TDEngine2::IWorld*                  mpWorld;
		
		TDEngine2::CEntity*                 mpCameraEntity;

		TDEngine2::IDesktopInputContext*    mpInputContext;
};