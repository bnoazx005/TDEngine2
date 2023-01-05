#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


#if 1 /// \fixme Fix these tests, Create X Component throws exception
TEST_CASE("CBaseComponentCloneable Tests")
{
	E_RESULT_CODE result = RC_OK;

	SECTION("TestClone_TryCloneTransform_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateTransform(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		const TVector3 expectedPosition = RandVector3();
		const TVector3 expectedRotation = TVector3(45.0f, 0.0f, 0.0f);
		const TVector3 expectedScale    = RandVector3();

		const TEntityId parentId = TEntityId(2);

		if (auto pTransform = DynamicPtrCast<ITransform>(pSourceComponent))
		{
			pTransform->SetPosition(expectedPosition);
			pTransform->SetRotation(TQuaternion(expectedRotation * CMathConstants::Deg2Rad));
			pTransform->SetScale(expectedScale);
			pTransform->SetParent(parentId);
		}

		auto pDestComponent = TPtr<IComponent>(CreateTransform(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));

		if (auto pTransform = DynamicPtrCast<ITransform>(pDestComponent))
		{
			REQUIRE(expectedPosition == pTransform->GetPosition());
			REQUIRE(TQuaternion(expectedRotation * CMathConstants::Deg2Rad) == pTransform->GetRotation());
			REQUIRE(expectedScale == pTransform->GetScale());
			REQUIRE(parentId == pTransform->GetParent());
		}
	}

#if 0
	SECTION("TestClone_TryCloneQuadSprite_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateQuadSprite(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateQuadSprite(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryClonePerspectiveCamera_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreatePerspectiveCamera(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreatePerspectiveCamera(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneOrthoCamera_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateOrthoCamera(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateOrthoCamera(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneBoxCollisionObject2D_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateBoxCollisionObject2D(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateBoxCollisionObject2D(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneCircleCollisionObject2D_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateCircleCollisionObject2D(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateCircleCollisionObject2D(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneTrigger2D_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateTrigger2D(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateTrigger2D(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneMeshAnimator_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateMeshAnimatorComponent(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateMeshAnimatorComponent(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneStaticMeshContainer_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateStaticMeshContainer(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateStaticMeshContainer(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneSkinnedMeshContainer_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateSkinnedMeshContainer(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateSkinnedMeshContainer(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneBoxCollisionObject3D_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateBoxCollisionObject3D(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateBoxCollisionObject3D(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneSphereCollisionObject3D_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateSphereCollisionObject3D(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateSphereCollisionObject3D(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneConvexHullCollisionObject3D_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateConvexHullCollisionObject3D(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateConvexHullCollisionObject3D(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneBoxCollisionObject3D_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateBoxCollisionObject3D(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateBoxCollisionObject3D(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneTrigger3D_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateTrigger3D(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateTrigger3D(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneBounds_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateBoundsComponent(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateBoundsComponent(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneDirectionalLight_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateDirectionalLight(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateDirectionalLight(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryClonePointLight_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreatePointLight(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreatePointLight(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneShadowCaster_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateShadowCasterComponent(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateShadowCasterComponent(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneShadowReceiver_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateShadowReceiverComponent(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateShadowReceiverComponent(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneAudioListener_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateAudioListenerComponent(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateAudioListenerComponent(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneAudioSource_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateAudioSourceComponent(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateAudioSourceComponent(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneAnimationContainer_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateAnimationContainerComponent(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateAnimationContainerComponent(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneSkybox_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateSkyboxComponent(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateSkyboxComponent(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneParticleEmitter_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateParticleEmitter(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateParticleEmitter(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneLayoutElement_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateLayoutElement(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateLayoutElement(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneCanvas_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateCanvas(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateCanvas(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneUIElementMeshData_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateUIElementMeshData(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateUIElementMeshData(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneImage_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateImage(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateImage(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneInputReceiver_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateInputReceiver(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateInputReceiver(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneLabel_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateLabel(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateLabel(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryClone9SliceImage_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(Create9SliceImage(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(Create9SliceImage(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneGridGroupLayout_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateGridGroupLayout(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateGridGroupLayout(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}

	SECTION("TestClone_TryCloneLODStrategyComponent_ANewInstanceCreatedWithSameParametersValues")
	{
		auto pSourceComponent = TPtr<IComponent>(CreateLODStrategyComponent(result));
		REQUIRE((pSourceComponent && RC_OK == result));

		auto pDestComponent = TPtr<IComponent>(CreateLODStrategyComponent(result));
		REQUIRE((pDestComponent && RC_OK == result));

		IComponent* pDestPtr = pDestComponent.Get();
		REQUIRE(RC_OK == pSourceComponent->Clone(pDestPtr));
	}
	#endif
}
#endif