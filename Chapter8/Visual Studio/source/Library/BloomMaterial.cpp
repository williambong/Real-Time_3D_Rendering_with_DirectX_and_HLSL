#include "BloomMaterial.h"
#include "GameException.h"

namespace Library
{
	RTTI_DEFINITIONS(BloomMaterial)

	BloomMaterial::BloomMaterial()
		: PostProcessingMaterial(),
		  MATERIAL_VARIABLE_INITIALIZATION(BloomTexture), MATERIAL_VARIABLE_INITIALIZATION(BloomThreshold),
		  MATERIAL_VARIABLE_INITIALIZATION(BloomIntensity), MATERIAL_VARIABLE_INITIALIZATION(BloomSaturation),
		  MATERIAL_VARIABLE_INITIALIZATION(SceneIntensity), MATERIAL_VARIABLE_INITIALIZATION(SceneSaturation)
	{
	}

	MATERIAL_VARIABLE_DEFINITION(BloomMaterial, BloomTexture)
	MATERIAL_VARIABLE_DEFINITION(BloomMaterial, BloomThreshold)
	MATERIAL_VARIABLE_DEFINITION(BloomMaterial, BloomIntensity)
	MATERIAL_VARIABLE_DEFINITION(BloomMaterial, BloomSaturation)
	MATERIAL_VARIABLE_DEFINITION(BloomMaterial, SceneIntensity)
	MATERIAL_VARIABLE_DEFINITION(BloomMaterial, SceneSaturation)

	void BloomMaterial::Initialize(Effect& effect)
	{
		PostProcessingMaterial::Initialize(effect);

		MATERIAL_VARIABLE_RETRIEVE(BloomTexture)
		MATERIAL_VARIABLE_RETRIEVE(BloomThreshold)
		MATERIAL_VARIABLE_RETRIEVE(BloomIntensity)
		MATERIAL_VARIABLE_RETRIEVE(BloomSaturation)
		MATERIAL_VARIABLE_RETRIEVE(SceneIntensity)
		MATERIAL_VARIABLE_RETRIEVE(SceneSaturation)
	}
}