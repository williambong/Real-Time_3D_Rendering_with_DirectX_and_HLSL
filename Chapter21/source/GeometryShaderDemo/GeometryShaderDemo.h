#pragma once

#include "DrawableGameComponent.h"
#include "RenderStateHelper.h"

using namespace Library;

namespace Library
{
	class Keyboard;
    class Effect;
	class Pass;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
	class PointSpriteMaterial;

    class GeometryShaderDemo : public DrawableGameComponent
    {
        RTTI_DECLARATIONS(GeometryShaderDemo, DrawableGameComponent)

    public:
        GeometryShaderDemo(Game& game, Camera& camera);
        ~GeometryShaderDemo();

        virtual void Initialize() override;
        virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

    private:
		void InitializeRandomPoints();
		void InitializeFixedPoints();

        GeometryShaderDemo();
        GeometryShaderDemo(const GeometryShaderDemo& rhs);
        GeometryShaderDemo& operator=(const GeometryShaderDemo& rhs);	

		Keyboard* mKeyboard;
        Effect* mEffect;
		PointSpriteMaterial* mMaterial;
		Pass* mPass;
		ID3D11InputLayout* mInputLayout;
		ID3D11Buffer* mVertexBuffer;
		UINT mVertexCount;
		ID3D11ShaderResourceView* mColorTexture;
		
		RenderStateHelper mRenderStateHelper;
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
		bool mShowRandomPoints;
    };
}
