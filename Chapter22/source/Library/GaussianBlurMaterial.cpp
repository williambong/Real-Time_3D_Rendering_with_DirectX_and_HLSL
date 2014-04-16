#include "GaussianBlurMaterial.h"
#include "GameException.h"

namespace Library
{
	RTTI_DEFINITIONS(GaussianBlurMaterial)

	GaussianBlurMaterial::GaussianBlurMaterial()
		: PostProcessingMaterial(),
		  MATERIAL_VARIABLE_INITIALIZATION(SampleOffsets), MATERIAL_VARIABLE_INITIALIZATION(SampleWeights)
	{
	}

	MATERIAL_VARIABLE_DEFINITION(GaussianBlurMaterial, SampleOffsets)
	MATERIAL_VARIABLE_DEFINITION(GaussianBlurMaterial, SampleWeights)

	void GaussianBlurMaterial::Initialize(Effect& effect)
	{
		PostProcessingMaterial::Initialize(effect);

		MATERIAL_VARIABLE_RETRIEVE(SampleOffsets)
		MATERIAL_VARIABLE_RETRIEVE(SampleWeights)
	}
}