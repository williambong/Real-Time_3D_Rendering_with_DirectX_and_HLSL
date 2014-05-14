#pragma once

#include "Common.h"

struct aiNodeAnim;

namespace Library
{
	class Model;
	class Bone;
	class Keyframe;

    class BoneAnimation
    {
		friend class AnimationClip;

    public:        
        ~BoneAnimation();
		
		Bone& GetBone();
		const std::vector<Keyframe*> Keyframes() const;

		UINT GetTransform(float time, XMFLOAT4X4& transform) const;
		void GetTransformAtKeyframe(UINT keyframeIndex, XMFLOAT4X4& transform) const;
		void GetInteropolatedTransform(float time, XMFLOAT4X4& transform) const;		

    private:
		BoneAnimation(Model& model, aiNodeAnim& nodeAnim);

		BoneAnimation();
        BoneAnimation(const BoneAnimation& rhs);
        BoneAnimation& operator=(const BoneAnimation& rhs);

		UINT FindKeyframeIndex(float time) const;

		Model* mModel;
		Bone* mBone;
		std::vector<Keyframe*> mKeyframes;
    };
}
