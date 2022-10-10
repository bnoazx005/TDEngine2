/*!
	\file CBoxCollisionObject2D.h
	\date 09.02.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseCollisionObject2D.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CBoxCollisionObject2D's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBoxCollisionObject2D's implementation
	*/

	TDE2_API IComponent* CreateBoxCollisionObject2D(E_RESULT_CODE& result);


	/*!
		class CBoxCollisionObject2D

		\brief The interface describes a functionality of a 2d box collision object
		which is controlled by Box2D physics engine
	*/

	class CBoxCollisionObject2D : public CBaseCollisionObject2D, public CPoolMemoryAllocPolicy<CBoxCollisionObject2D, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateBoxCollisionObject2D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CBoxCollisionObject2D)

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			/*!
				\brief The method creates a new deep copy of the instance and returns a smart pointer to it.
				The original state of the object stays the same

				\param[in] pDestObject A valid pointer to an object which the properties will be assigned into
			*/

			TDE2_API E_RESULT_CODE Clone(IComponent*& pDestObject) const override;

			/*!
				\brief The method sets up a width of a box collider

				\param[in] width A width of a box collider
			*/

			TDE2_API virtual void SetWidth(F32 width);

			/*!
				\brief The method sets up a height of a box collider

				\param[in] width A height of a box collider
			*/

			TDE2_API virtual void SetHeight(F32 height);

			/*!
				\brief The method returns a width of a box collider

				\return The method returns a width of a box collider
			*/

			TDE2_API virtual F32 GetWidth() const;

			/*!
				\brief The method returns a height of a box collider

				\return The method returns a height of a box collider
			*/

			TDE2_API virtual F32 GetHeight() const;

			/*!
				\brief The method returns a rvalue reference to internal representation of a collision
				shape that is used by Box2D physics engine

				\param[in] pVisitor A pointer to implementation of ICollisionObjectsVisitor

				\param[in] callback A callback that is called when a new definition of a shape was created
			*/

			TDE2_API void GetCollisionShape(const ICollisionObjectsVisitor* pVisitor, const std::function<void(const b2Shape*)>& callback) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBoxCollisionObject2D)
		protected:
			F32 mWidth;

			F32 mHeight;
	};


	/*!
		struct TBoxCollisionObject2DParameters

		\brief The structure contains parameters for creation of CBoxCollisionObject2D
	*/

	typedef struct TBoxCollisionObject2DParameters : public TBaseComponentParameters
	{
		F32 mWidth = 1.0f;
		F32 mHeight = 1.0f;
	} TBoxCollisionObject2DParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(BoxCollisionObject2D, TBoxCollisionObject2DParameters);
}