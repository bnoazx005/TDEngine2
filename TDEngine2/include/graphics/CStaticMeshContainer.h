/*!
	\file CStaticMeshContainer.h
	\date 07.11.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "IStaticMeshContainer.h"
#include "./../ecs/CBaseComponent.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CStaticMeshContainer's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CStaticMeshContainer's implementation
	*/

	TDE2_API IComponent* CreateStaticMeshContainer(E_RESULT_CODE& result);


	/*!
		interface CStaticMeshContainer

		\brief The interface describes a functionality of a container for 3d static meshes
		that stores a reference to a mesh resource, a material and other things
	*/

	class CStaticMeshContainer : public IStaticMeshContainer, public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateStaticMeshContainer(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CStaticMeshContainer)

			/*!
				\brief The method initializes an internal state of a quad sprite

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

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
				\brief The method sets up an identifier fo a material that will be used for the sprite

				\param[in] materialName A string that contains material's name

				\param[in] materialId An identifier of a material
			*/

			TDE2_API void SetMaterialName(const std::string& materialName) override;

			/*!
				\brief The method sets up an identifier fo a mesh that will be used

				\param[in] meshName A string that contains mesh's name
			*/

			TDE2_API void SetMeshName(const std::string& meshName) override;

			/*!
				\brief The method sets up an internal handle which points to pair
				vertex and index buffers within system that renders static meshes

				\param[in] handle An internal handle which points to pair
				vertex and index buffers within system that renders static meshes
			*/

			TDE2_API void SetSystemBuffersHandle(U32 handle) override;

			/*!
				\brief The method returns an identifier of used material

				\return The method returns an identifier of used material
			*/

			TDE2_API const std::string& GetMaterialName() const override;

			/*!
				\brief The method returns an identifier of used mesh

				\return The method returns an identifier of used mesh
			*/

			TDE2_API const std::string& GetMeshName() const override;

			/*!
				\brief The method returns an internal handle which points to pair
				vertex and index buffers within system that renders static meshes
			*/

			TDE2_API U32 GetSystemBuffersHandle() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CStaticMeshContainer)
		protected:
			std::string mMaterialName;
			std::string mMeshName; /// \todo replace with GUID or something like that

			U32         mSystemBuffersHandle = static_cast<U32>(-1);
	};


	/*!
		struct TStaticMeshContainerParameters

		\brief The structure contains parameters for creation of CStaticMeshContainer
	*/

	typedef struct TStaticMeshContainerParameters : public TBaseComponentParameters
	{
	} TStaticMeshContainerParameters;


	/*!
		\brief A factory function for creation objects of CStaticMeshContainerFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CStaticMeshContainerFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateStaticMeshContainerFactory(E_RESULT_CODE& result);


	/*!
		class CStaticMeshContainerFactory

		\brief The class is factory facility to create a new objects of CStaticMeshContainer type
	*/

	class CStaticMeshContainerFactory : public IStaticMeshContainerFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateStaticMeshContainerFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* Create(const TBaseComponentParameters* pParams) const override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* CreateDefault(const TBaseComponentParameters& params) const override;

			/*!
				\brief The method returns an identifier of a component's type, which
				the factory serves

				\return The method returns an identifier of a component's type, which
				the factory serves
			*/

			TDE2_API TypeId GetComponentTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CStaticMeshContainerFactory)
	};
}