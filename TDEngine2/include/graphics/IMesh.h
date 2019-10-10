/*!
	\file IMesh.h
	\date 10.10.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../ecs/IComponentFactory.h"
#include <string>


namespace TDEngine2
{
	/*!
		interface IMesh

		\brief The interface describes a functionality of a common mesh
	*/

	class IMesh : public virtual IBaseObject
	{
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IMesh)
	};


	/*!
		interface IStaticMesh

		\brief The interface describes a functionality of static meshes
	*/

	class IStaticMesh : public IMesh
	{
		public:
			/*!
				\brief The method initializes an internal state of a quad sprite

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IStaticMesh)
	};


	/*!
		interface IStaticMeshFactory

		\brief The interface represents a functionality of a factory of IStaticMesh objects
	*/

	class IStaticMeshFactory : public IComponentFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
		protected:
	};
}