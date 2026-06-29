/*!
	\file CCanvasComponent.h
	\date 28.04.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../math/TMatrix4.h"


namespace TDEngine2
{
	enum class TResourceId : U32;


	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TCanvasComponentData
	{
		FIELD_META(name = width) U32                 mWidth = 1280;
		FIELD_META(name = height) U32                mHeight = 720;

		bool                                         mIsDirty = true;
		FIELD_META(name = inherit_camera_sizes) bool mInheritsSizesFromMainCamera;

		TMatrix4                                     mProjMatrix = IdentityMatrix4;

		FIELD_META(name = priority) int              mPriority = 0;

		TDE2_DECLARE_COMPONENT_META(TCanvasComponentData);
	};


	/*!
		\brief A factory function for creation objects of CCanvas's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CCanvas's implementation
	*/

	TDE2_API IComponent* CreateCanvas(E_RESULT_CODE& result);


	/*!
		class CCanvas

		\brief The class is a main element that determines sizes of UI's field
	*/

	class CCanvas : public CBaseComponentT<CCanvas, TCanvasComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateCanvas(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CCanvas)

			TDE2_API void SetWidth(U32 value);
			TDE2_API void SetHeight(U32 value);

			TDE2_API void SetProjMatrix(const TMatrix4& projMat);

			TDE2_API void SetInheritSizesFromMainCamera(bool value);

			TDE2_API void SetPriority(int value);

			TDE2_API U32 GetWidth() const;
			TDE2_API U32 GetHeight() const;

			TDE2_API bool IsDirty() const;
			TDE2_API bool DoesInheritSizesFromMainCamera() const;

			TDE2_API const TMatrix4& GetProjMatrix() const;

			TDE2_API int GetPriority() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CCanvas)
	};


	/*!
		struct TCanvasParameters

		\brief The structure contains parameters for creation of CCanvas
	*/

	typedef struct TCanvasParameters : public TBaseComponentParameters
	{
	} TCanvasParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(Canvas, TCanvasParameters);
}