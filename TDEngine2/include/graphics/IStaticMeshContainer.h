/*!
	\file IStaticMeshContainer.h
	\date 07.11.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../ecs/IComponentFactory.h"
#include "./../graphics/IRenderable.h"
#include "./../utils/Color.h"
#include <string>


namespace TDEngine2
{
	/*!
		interface IStaticMeshContainer

		\brief The interface describes a functionality of a holder of a 3D static mesh
	*/

	class IStaticMeshContainer : public IRenderable
	{
		public:
			/*!
				\brief The method initializes an internal state of a component

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method sets up an identifier fo a material that will be used for the mesh's instance

				\param[in] materialName A string that contains material's name
			*/

			TDE2_API virtual void SetMaterialName(const std::string& materialName) = 0;

			/*!
				\brief The method sets up an identifier fo a mesh that will be used

				\param[in] meshName A string that contains mesh's name
			*/

			TDE2_API virtual void SetMeshName(const std::string& meshName) = 0;

			/*!
				\brief The method returns an identifier of used material

				\return The method returns an identifier of used material
			*/

			TDE2_API virtual const std::string& GetMaterialName() const = 0;

			/*!
				\brief The method returns an identifier of used mesh

				\return The method returns an identifier of used mesh
			*/

			TDE2_API virtual const std::string& GetMeshName() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IStaticMeshContainer)
	};


	/*!
		interface IStaticMeshContainerFactory

		\brief The interface represents a functionality of a factory of IStaticMeshContainer objects
	*/

	class IStaticMeshContainerFactory : public IComponentFactory
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