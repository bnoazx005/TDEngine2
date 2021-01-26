/*!
	\file CAnimationClip.h
	\date 15.04.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "IAnimationClip.h"
#include "../../core/CBaseResource.h"
#include "../../core/IResourceLoader.h"
#include <vector>
#include <functional>
#include <unordered_map>


namespace TDEngine2
{
	class IAnimationTrack;


	/*!
		\brief A factory function for creation objects of CAnimationClip's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation
		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[in] name A resource's name
		\param[in] params A parameters of created animation clip
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAnimationClip's implementation
	*/

	TDE2_API IAnimationClip* CreateAnimationClip(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												const TAnimationClipParameters& params, E_RESULT_CODE& result);

	/*!
		\brief A factory function for creation objects of CAnimationClip's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAnimationClip's implementation
	*/

	TDE2_API IAnimationClip* CreateAnimationClip(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												 E_RESULT_CODE& result);


	/*!
		class CAnimationClip

		\brief The implementation represents a functionality of animation clips
	*/

	class CAnimationClip: public CBaseResource, public IAnimationClip
	{
		public:
			friend TDE2_API IAnimationClip* CreateAnimationClip(IResourceManager*, IGraphicsContext*, const std::string&, const TAnimationClipParameters&, E_RESULT_CODE&);
			friend TDE2_API IAnimationClip* CreateAnimationClip(IResourceManager*, IGraphicsContext*, const std::string&, E_RESULT_CODE&);
		public:
			typedef std::unordered_map<TAnimationTrackId, IAnimationTrack*> TAnimationTracks;
			typedef std::function<IAnimationTrack*(IAnimationClip*)> TAnimationTrackFactoryFunctor;
			typedef std::unordered_map<TypeId, TAnimationTrackFactoryFunctor> TAnimationTracksFactory;
		public:
			TDE2_REGISTER_RESOURCE_TYPE(CAnimationClip)
			TDE2_REGISTER_TYPE(CAnimationClip)

			/*!
				\brief The method initializes a state of a brand new animation clip.

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] name A resource's name
				\param[in] params Additional parameters of a clip

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
										const TAnimationClipParameters& params) override;

			/*!
				\brief The method initializes an internal state of an animation clip

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) override;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;

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

			TDE2_API E_RESULT_CODE RemoveTrack(TAnimationTrackId handle) override;

			/*!
				\brief The method specifies duration of the clip

				\param[in] duration A time length of the animation clip, should be positive value

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetDuration(F32 duration) override;

			/*!
				\brief The method specified wrapping mode for the clip. Remember that this value probably would
				be overwritten when the clip would be assigned into animation component

				\param[in] mode The value determines how the clip would be played
			*/

			TDE2_API void SetWrapMode(E_ANIMATION_WRAP_MODE_TYPE mode) override;

			/*!
				\brief The method returns duration of the animation clip
				\return The method returns duration of the animation clip
			*/

			TDE2_API F32 GetDuration() const override;

			/*!
				\brief The method returns wrap mode that's defined for the clip
				\return The method returns wrap mode that's defined for the clip
			*/

			TDE2_API E_ANIMATION_WRAP_MODE_TYPE GetWrapMode() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAnimationClip)

			TDE2_API const IResourceLoader* _getResourceLoader() override;

			TDE2_API TAnimationTrackId _createTrackInternal(TypeId typeId, const std::string& name) override;

			TDE2_API IAnimationTrack* _getTrackInternal(TAnimationTrackId handle) override;
		protected:
			static const TAnimationTracksFactory mTracksFactory;

			static constexpr U16       mVersionTag = 0x1;
			
			TAnimationTracks           mpTracks;

			F32                        mDuration;

			E_ANIMATION_WRAP_MODE_TYPE mWrapMode;
	};


	/*!
		\brief A factory function for creation objects of CAnimationClipLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in, out] pFileSystem A pointer to IFileSystem's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAnimationClipLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateAnimationClipLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
														E_RESULT_CODE& result);


	/*!
		class CAnimationClipLoader

		\brief The class is an implementation of a loader of animation clips
	*/

	class CAnimationClipLoader : public CBaseObject, public IAnimationClipLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateAnimationClipLoader(IResourceManager*, IGraphicsContext*, IFileSystem*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method loads data into the specified resource based on its
				internal information

				\param[in, out] pResource A pointer to an allocated resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadResource(IResource* pResource) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the loader serves

				\return The method returns an identifier of a resource's type, which
				the loader serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAnimationClipLoader)
		protected:
			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;

			IGraphicsContext* mpGraphicsContext;
	};


	/*!
		\brief A factory function for creation objects of CAnimationClipFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAnimationClipFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateAnimationClipFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CAnimationClipFactory

		\brief The class is an implementation of a factory of animation clips
	*/

	class CAnimationClipFactory : public CBaseObject, public IAnimationClipFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateAnimationClipFactory(IResourceManager*, IGraphicsContext*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* Create(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAnimationClipFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}