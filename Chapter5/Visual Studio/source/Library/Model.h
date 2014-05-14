#pragma once

#include "Common.h"

struct aiNode;

namespace Library
{
    class Game;
    class Mesh;
    class ModelMaterial;
	class AnimationClip;
	class SceneNode;
	class Bone;

    class Model
    {
		friend class Mesh;

    public:
        Model(Game& game, const std::string& filename, bool flipUVs = false);
        ~Model();

        Game& GetGame();
        bool HasMeshes() const;
        bool HasMaterials() const;
		bool HasAnimations() const;

        const std::vector<Mesh*>& Meshes() const;
        const std::vector<ModelMaterial*>& Materials() const;
		const std::vector<AnimationClip*>& Animations() const;
		const std::map<std::string, AnimationClip*>& AnimationsbyName() const;
		const std::vector<Bone*> Bones() const;
		const std::map<std::string, UINT> BoneIndexMapping() const;
		SceneNode* RootNode();

    private:
        Model(const Model& rhs);
        Model& operator=(const Model& rhs);

		SceneNode* BuildSkeleton(aiNode& node, SceneNode* parentSceneNode);
		void ValidateModel();
		void DeleteSceneNode(SceneNode* sceneNode);

        Game& mGame;
        std::vector<Mesh*> mMeshes;
        std::vector<ModelMaterial*> mMaterials;
		std::vector<AnimationClip*> mAnimations;
		std::map<std::string, AnimationClip*> mAnimationsByName;
		std::vector<Bone*> mBones;
		std::map<std::string, UINT> mBoneIndexMapping;
		SceneNode* mRootNode;
    };
}
