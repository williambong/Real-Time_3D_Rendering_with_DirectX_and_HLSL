#pragma once

#include "DrawableGameComponent.h"
#include "RenderStateHelper.h"

using namespace Library;

namespace Library
{
	class Keyboard;
	class PointLight;
	class ProxyModel;
    class Effect;
	class SkinnedModelMaterial;
	class Model;
	class AnimationPlayer;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
    class AnimationDemo : public DrawableGameComponent
    {
        RTTI_DECLARATIONS(AnimationDemo, DrawableGameComponent)

    public:
        AnimationDemo(Game& game, Camera& camera);
        ~AnimationDemo();

        virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
        virtual void Draw(const GameTime& gameTime) override;

    private:
        AnimationDemo();
        AnimationDemo(const AnimationDemo& rhs);
        AnimationDemo& operator=(const AnimationDemo& rhs);	

		void UpdateOptions();
		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdatePointLight(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);

		static const float LightModulationRate;
		static const float LightMovementRate;

        Effect* mEffect;
		SkinnedModelMaterial* mMaterial;

		Keyboard* mKeyboard;
		XMCOLOR mAmbientColor;
		PointLight* mPointLight;
		XMCOLOR mSpecularColor;
		float mSpecularPower;
        XMFLOAT4X4 mWorldMatrix;

		std::vector<ID3D11Buffer*> mVertexBuffers;
		std::vector<ID3D11Buffer*> mIndexBuffers;
		std::vector<UINT> mIndexCounts;
		std::vector<ID3D11ShaderResourceView*> mColorTextures;

		Model* mSkinnedModel;
		AnimationPlayer* mAnimationPlayer;		

		RenderStateHelper mRenderStateHelper;
		ProxyModel* mProxyModel;
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
		bool mManualAdvanceMode;
    };
}
