#pragma once

#include "Common.h"
#include "PostProcessingMaterial.h"

namespace Library
{
	class GaussianBlurMaterial : public PostProcessingMaterial
	{
		RTTI_DECLARATIONS(PostProcessingMaterial, GaussianBlurMaterial)

		MATERIAL_VARIABLE_DECLARATION(SampleOffsets)
		MATERIAL_VARIABLE_DECLARATION(SampleWeights)

	public:
		GaussianBlurMaterial();

		virtual void Initialize(Effect& effect) override;
	};
}

