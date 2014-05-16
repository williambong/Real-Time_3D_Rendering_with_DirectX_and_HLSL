#pragma once

#include "Common.h"
#include "PostProcessingMaterial.h"

namespace Library
{
	class BloomMaterial : public PostProcessingMaterial
	{
		RTTI_DECLARATIONS(PostProcessingMaterial, BloomMaterial)

		MATERIAL_VARIABLE_DECLARATION(BloomTexture)
		MATERIAL_VARIABLE_DECLARATION(BloomThreshold)
		MATERIAL_VARIABLE_DECLARATION(BloomIntensity)
		MATERIAL_VARIABLE_DECLARATION(BloomSaturation)
		MATERIAL_VARIABLE_DECLARATION(SceneIntensity)
		MATERIAL_VARIABLE_DECLARATION(SceneSaturation)

	public:
		BloomMaterial();

		virtual void Initialize(Effect& effect) override;
	};
}

