#include "BasicTessellationDemo.h"
#include "Game.h"
#include "GameException.h"
#include "Camera.h"
#include "Utility.h"
#include "Effect.h"
#include "Keyboard.h"
#include "TriangleTessellationMaterial.h"
#include "QuadTessellationMaterial.h"
#include "RasterizerStates.h"
#include <sstream>
#include <SpriteBatch.h>
#include <SpriteFont.h>

namespace Rendering
{
    RTTI_DEFINITIONS(BasicTessellationDemo)

	const float BasicTessellationDemo::MaxTessellationFactor = 64.0f;

    BasicTessellationDemo::BasicTessellationDemo(Game& game, Camera& camera)
        : DrawableGameComponent(game, camera),  
		  mKeyboard(nullptr), mTriMaterial(nullptr), mTriEffect(nullptr), mTriPass(nullptr), mTriInputLayout(nullptr), mTriVertexBuffer(nullptr),
		  mQuadMaterial(nullptr), mQuadEffect(nullptr), mQuadPass(nullptr), mQuadInputLayout(nullptr), mQuadVertexBuffer(nullptr), mRenderStateHelper(game),
		  mTessellationEdgeFactors(), mTessellationInsideFactors(), mUniformTessellation(true), mShowQuadTopology(false),
		  mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f)
    {
    }

    BasicTessellationDemo::~BasicTessellationDemo()
    {
		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);
		ReleaseObject(mQuadVertexBuffer);
		DeleteObject(mQuadMaterial);
		DeleteObject(mQuadEffect);
		ReleaseObject(mTriVertexBuffer);
        DeleteObject(mTriMaterial);
        DeleteObject(mTriEffect);
    }

    void BasicTessellationDemo::Initialize()
    {
        SetCurrentDirectory(Utility::ExecutableDirectory().c_str());
	
        // Initialize the materials
        mTriEffect = new Effect(*mGame);
        mTriEffect->LoadCompiledEffect(L"Content\\Effects\\TriTessellation.cso");
		mTriMaterial = new TriangleTessellationMaterial();
        mTriMaterial->Initialize(*mTriEffect);

		mTriPass = mTriMaterial->CurrentTechnique()->Passes().at(0);
		mTriInputLayout = mTriMaterial->InputLayouts().at(mTriPass);

		mQuadEffect = new Effect(*mGame);
		mQuadEffect->LoadCompiledEffect(L"Content\\Effects\\QuadTessellation.cso");
		mQuadMaterial = new QuadTessellationMaterial();
		mQuadMaterial->Initialize(*mQuadEffect);

		mQuadPass = mQuadMaterial->CurrentTechnique()->Passes().at(0);
		mQuadInputLayout = mQuadMaterial->InputLayouts().at(mQuadPass);

		// Create the vertex buffers
		VertexPosition triVertices[] =
		{
			VertexPosition(XMFLOAT4(-5.0f, 1.0f, 0.0f, 1.0f)),
			VertexPosition(XMFLOAT4(0.0f, 6.0f, 0.0f, 1.0f)),
			VertexPosition(XMFLOAT4(5.0f, 1.0f, 0.0f, 1.0f))
		};

		mTriMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), triVertices, ARRAYSIZE(triVertices), &mTriVertexBuffer);

		VertexPosition quadVertices[] =
		{
			VertexPosition(XMFLOAT4(-5.0f, 6.0f, 0.0f, 1.0f)),
			VertexPosition(XMFLOAT4(5.0f, 6.0f, 0.0f, 1.0f)),
			VertexPosition(XMFLOAT4(-5.0f, 1.0f, 0.0f, 1.0f)),
			VertexPosition(XMFLOAT4(5.0f, 1.0f, 0.0f, 1.0f))
		};

		mQuadMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), quadVertices, ARRAYSIZE(quadVertices), &mQuadVertexBuffer);

		mTessellationEdgeFactors.push_back(2);
		mTessellationEdgeFactors.push_back(2);
		mTessellationEdgeFactors.push_back(2);
		mTessellationEdgeFactors.push_back(2);

		mTessellationInsideFactors.push_back(2);
		mTessellationInsideFactors.push_back(2);

		mKeyboard = (Keyboard*)mGame->Services().GetService(Keyboard::TypeIdClass());
		assert(mKeyboard != nullptr);

		mSpriteBatch = new SpriteBatch(mGame->Direct3DDeviceContext());
		mSpriteFont = new SpriteFont(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");
    }

	void BasicTessellationDemo::Update(const GameTime& gameTime)
	{
		if (mKeyboard->WasKeyPressedThisFrame(DIK_SPACE))
		{
			mUniformTessellation = !mUniformTessellation;
		}

		if (mKeyboard->WasKeyPressedThisFrame(DIK_T))
		{
			mShowQuadTopology = !mShowQuadTopology;
		}

		if (mUniformTessellation)
		{
			if (mKeyboard->WasKeyPressedThisFrame(DIK_UPARROW))
			{
				float tesselletionFactor = mTessellationEdgeFactors[0] + 1;
				tesselletionFactor = min(tesselletionFactor, MaxTessellationFactor);

				for (UINT i = 0; i < mTessellationEdgeFactors.size(); i++)
				{
					mTessellationEdgeFactors[i] = tesselletionFactor;
				}

				for (UINT i = 0; i < mTessellationInsideFactors.size(); i++)
				{
					mTessellationInsideFactors[i] = tesselletionFactor;
				}
			}

			if (mKeyboard->WasKeyPressedThisFrame(DIK_DOWNARROW))
			{
				float tesselletionFactor = mTessellationEdgeFactors[0] - 1;
				tesselletionFactor = max(tesselletionFactor, 1);

				for (UINT i = 0; i < mTessellationEdgeFactors.size(); i++)
				{
					mTessellationEdgeFactors[i] = tesselletionFactor;
				}

				for (UINT i = 0; i < mTessellationInsideFactors.size(); i++)
				{
					mTessellationInsideFactors[i] = tesselletionFactor;
				}
			}
		}
		else
		{
			if (mKeyboard->WasKeyPressedThisFrame(DIK_U))
			{
				float tesselletionFactor = mTessellationEdgeFactors[0] + 1;
				mTessellationEdgeFactors[0] = min(tesselletionFactor, MaxTessellationFactor);
			}

			if (mKeyboard->WasKeyPressedThisFrame(DIK_H))
			{
				float tesselletionFactor = mTessellationEdgeFactors[0] - 1;
				mTessellationEdgeFactors[0] = max(tesselletionFactor, 1);
			}

			if (mKeyboard->WasKeyPressedThisFrame(DIK_I))
			{
				float tesselletionFactor = mTessellationEdgeFactors[1] + 1;
				mTessellationEdgeFactors[1] = min(tesselletionFactor, MaxTessellationFactor);
			}

			if (mKeyboard->WasKeyPressedThisFrame(DIK_J))
			{
				float tesselletionFactor = mTessellationEdgeFactors[1] - 1;
				mTessellationEdgeFactors[1] = max(tesselletionFactor, 1);
			}

			if (mKeyboard->WasKeyPressedThisFrame(DIK_O))
			{
				float tesselletionFactor = mTessellationEdgeFactors[2] + 1;
				mTessellationEdgeFactors[2] = min(tesselletionFactor, MaxTessellationFactor);
			}

			if (mKeyboard->WasKeyPressedThisFrame(DIK_K))
			{
				float tesselletionFactor = mTessellationEdgeFactors[2] - 1;
				mTessellationEdgeFactors[2] = max(tesselletionFactor, 1);
			}

			if (mShowQuadTopology && mKeyboard->WasKeyPressedThisFrame(DIK_P))
			{
				float tesselletionFactor = mTessellationEdgeFactors[3] + 1;
				mTessellationEdgeFactors[3] = min(tesselletionFactor, MaxTessellationFactor);
			}

			if (mShowQuadTopology && mKeyboard->WasKeyPressedThisFrame(DIK_L))
			{
				float tesselletionFactor = mTessellationEdgeFactors[3] - 1;
				mTessellationEdgeFactors[3] = max(tesselletionFactor, 1);
			}

			if (mKeyboard->WasKeyPressedThisFrame(DIK_V))
			{
				float tesselletionFactor = mTessellationInsideFactors[0] + 1;
				mTessellationInsideFactors[0] = min(tesselletionFactor, MaxTessellationFactor);
			}

			if (mKeyboard->WasKeyPressedThisFrame(DIK_B))
			{
				float tesselletionFactor = mTessellationInsideFactors[0] - 1;
				mTessellationInsideFactors[0] = max(tesselletionFactor, 1);
			}

			if (mShowQuadTopology && mKeyboard->WasKeyPressedThisFrame(DIK_N))
			{
				float tesselletionFactor = mTessellationInsideFactors[1] + 1;
				mTessellationInsideFactors[1] = min(tesselletionFactor, MaxTessellationFactor);
			}

			if (mShowQuadTopology && mKeyboard->WasKeyPressedThisFrame(DIK_M))
			{
				float tesselletionFactor = mTessellationInsideFactors[1] - 1;
				mTessellationInsideFactors[1] = max(tesselletionFactor, 1);
			}
		}
	}

	void BasicTessellationDemo::Draw(const GameTime& gameTime)
	{
		mRenderStateHelper.SaveRasterizerState();

		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->RSSetState(RasterizerStates::Wireframe);

		if (mShowQuadTopology)
		{
			direct3DDeviceContext->IASetInputLayout(mQuadInputLayout);
			direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

			UINT stride = mQuadMaterial->VertexSize();
			UINT offset = 0;
			direct3DDeviceContext->IASetVertexBuffers(0, 1, &mQuadVertexBuffer, &stride, &offset);

			mQuadMaterial->WorldViewProjection() << mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
			mQuadMaterial->TessellationEdgeFactors() << mTessellationEdgeFactors;
			mQuadMaterial->TessellationInsideFactors() << mTessellationInsideFactors;
			mQuadPass->Apply(0, direct3DDeviceContext);

			direct3DDeviceContext->Draw(4, 0);
		}
		else
		{
			direct3DDeviceContext->IASetInputLayout(mTriInputLayout);
			direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

			UINT stride = mTriMaterial->VertexSize();
			UINT offset = 0;
			direct3DDeviceContext->IASetVertexBuffers(0, 1, &mTriVertexBuffer, &stride, &offset);

			std::vector<float> tessellationEdgeFactors(mTessellationEdgeFactors.begin(), mTessellationEdgeFactors.end() - 1);

			mTriMaterial->WorldViewProjection() << mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
			mTriMaterial->TessellationEdgeFactors() << tessellationEdgeFactors;
			mTriMaterial->TessellationInsideFactor() << mTessellationInsideFactors[0];
			mTriPass->Apply(0, direct3DDeviceContext);

			direct3DDeviceContext->Draw(3, 0);
		}

		direct3DDeviceContext->HSSetShader(nullptr, nullptr, 0);
		direct3DDeviceContext->DSSetShader(nullptr, nullptr, 0);

		mRenderStateHelper.RestoreAll();

		mRenderStateHelper.SaveAll();
		mSpriteBatch->Begin();

		std::wostringstream helpLabel;
		helpLabel << "Topology (T): " << (mShowQuadTopology ? "Quad" : "Tri") << "\nUniform Tessellation (Space): " << (mUniformTessellation ? "True" : "False");

		if (mUniformTessellation)
		{
			if (mShowQuadTopology)
			{
				helpLabel << "\nTessellation Edge Factors: (+Up/-Down)[" << mTessellationEdgeFactors[0] << ", " << mTessellationEdgeFactors[1] << ", " << mTessellationEdgeFactors[2] << ", " << mTessellationEdgeFactors[3] << "]"
					<< "\nTessellation Inside Factor: [" << mTessellationInsideFactors[0] << ", " << mTessellationInsideFactors[1] << "]";
			}
			else
			{
				helpLabel << "\nTessellation Edge Factors: (+Up/-Down)[" << mTessellationEdgeFactors[0] << ", " << mTessellationEdgeFactors[1] << ", " << mTessellationEdgeFactors[2] << "]"
					<< "\nTessellation Inside Factor: [" << mTessellationInsideFactors[0] << "]";
			}
		}
		else
		{
			if (mShowQuadTopology)
			{
				helpLabel << "\nTessellation Edge Factors: (+U/-H, +I/-J, +O/-K, +P/-L)[" << mTessellationEdgeFactors[0] << ", " << mTessellationEdgeFactors[1] << ", " << mTessellationEdgeFactors[2] << ", " << mTessellationEdgeFactors[3] << "]"
					<< "\nTessellation Inside Factor: (+V/-B, +N/-M)  [" << mTessellationInsideFactors[0] << ", " << mTessellationInsideFactors[1] << "]";
			}
			else
			{
				helpLabel << "\nTessellation Edge Factors: (+U/-H, +I/-J, +O/-K)[" << mTessellationEdgeFactors[0] << ", " << mTessellationEdgeFactors[1] << ", " << mTessellationEdgeFactors[2] << "]"
					<< "\nTessellation Inside Factor: (+V/-B)  [" << mTessellationInsideFactors[0] << "]";
			}
		}

		mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();
		mRenderStateHelper.RestoreAll();
    }
}