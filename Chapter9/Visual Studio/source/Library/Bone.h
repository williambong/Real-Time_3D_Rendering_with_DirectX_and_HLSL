#pragma once

#include "Common.h"
#include "SceneNode.h"

namespace Library
{
	class BoneVertexWeights
	{
	public:
		typedef struct _VertexWeight
		{
			float Weight;
			UINT BoneIndex;

			_VertexWeight(float weight, UINT boneIndex)
				: Weight(weight), BoneIndex(boneIndex) { }
		} VertexWeight;
		
		const std::vector<VertexWeight>& Weights();

		void AddWeight(float weight, UINT boneIndex);

		static const UINT MaxBoneWeightsPerVertex = 4U;

	private:
		std::vector<VertexWeight> mWeights;
	};

    class Bone : public SceneNode
    {
		RTTI_DECLARATIONS(Bone, SceneNode)

    public:	
		UINT Index() const;
		void SetIndex(UINT index);

		const XMFLOAT4X4& OffsetTransform() const;
		XMMATRIX OffsetTransformMatrix() const;

		Bone(const std::string& name, UINT index, const XMFLOAT4X4& offsetTransform);

    private:
		Bone();
        Bone(const Bone& rhs);
        Bone& operator=(const Bone& rhs);

		UINT mIndex;					// Index into the model's bone container
		XMFLOAT4X4 mOffsetTransform;	// Transforms from mesh space to bone space
    };
}
