#include "ModelTessellationDemo.h"
#include "Game.h"
#include "GameException.h"
#include "Camera.h"
#include "Utility.h"
#include "Effect.h"
#include "Keyboard.h"
#include "ModelTessellationMaterial.h"
#include "RasterizerStates.h"
#include "Model.h"
#include "Mesh.h"
#include <sstream>
#include <iomanip>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>
#include "Grid.h"
#include "MatrixHelper.h"

namespace Rendering
{
    RTTI_DEFINITIONS(ModelTessellationDemo)

    ModelTessellationDemo::ModelTessellationDemo(Game& game, Camera& camera)
        : DrawableGameComponent(game, camera),  
		  mKeyboard(nullptr), mMaterial(nullptr), mEffect(nullptr), mPass(nullptr), mInputLayout(nullptr), mVertexBuffer(nullptr), mIndexBuffer(nullptr),
		  mIndexCount(0), mColorTexture(nullptr), mWorldMatrix(MatrixHelper::Identity),
		  mRenderStateHelper(game), mTessellationEdgeFactors(), mTessellationInsideFactor(2),
		  mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f), mShowWireframe(true), mDistanceModeEnabled(false),
		  mMaxTessellationFactor(64), mMinTessellationDistance(2.0f), mMaxTessellationDistance(20.0f)
    {
    }

    ModelTessellationDemo::~ModelTessellationDemo()
    {
		ReleaseObject(mColorTexture);
		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);
		ReleaseObject(mIndexBuffer);
		ReleaseObject(mVertexBuffer);
		DeleteObject(mMaterial);
		DeleteObject(mEffect);
    }

    void ModelTessellationDemo::Initialize()
    {
        SetCurrentDirectory(Utility::ExecutableDirectory().c_str());
	
        // Initialize the material
		mEffect = new Effect(*mGame);
		mEffect->LoadCompiledEffect(L"Content\\Effects\\ModelTessellation.cso");
		mMaterial = new ModelTessellationMaterial();
		mMaterial->Initialize(*mEffect);

		Technique* technique = mEffect->TechniquesByName().at("solid_color_manual_factors");
		mMaterial->SetCurrentTechnique(*technique);
		mPass = mMaterial->CurrentTechnique()->Passes().at(0);
		mInputLayout = mMaterial->InputLayouts().at(mPass);

		// Load the model
		std::unique_ptr<Model> model(new Model(*mGame, "Content\\Models\\Sphere.obj", true));

		// Create the vertex and index buffers
		Mesh* mesh = model->Meshes().at(0);
		mMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), *mesh, &mVertexBuffer);
		mesh->CreateIndexBuffer(&mIndexBuffer);
		mIndexCount = mesh->Indices().size();

		// Load the texture
		HRESULT hr;
		std::wstring textureName = L"Content\\Textures\\EarthComposite.jpg";
		if (FAILED(hr = DirectX::CreateWICTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), textureName.c_str(), nullptr, &mColorTexture)))
		{
			throw GameException("CreateWICTextureFromFile() failed.", hr);
		}

		mTessellationEdgeFactors.push_back(2);
		mTessellationEdgeFactors.push_back(2);
		mTessellationEdgeFactors.push_back(2);

		mKeyboard = (Keyboard*)mGame->Services().GetService(Keyboard::TypeIdClass());
		assert(mKeyboard != nullptr);

		mSpriteBatch = new SpriteBatch(mGame->Direct3DDeviceContext());
		mSpriteFont = new SpriteFont(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");
    }

	void ModelTessellationDemo::Update(const GameTime& gameTime)
	{
		if (mKeyboard->WasKeyPressedThisFrame(DIK_SPACE))
		{
			mDistanceModeEnabled = !mDistanceModeEnabled;
			UpdateTechnique();
		}

		if (mKeyboard->WasKeyPressedThisFrame(DIK_RETURN))
		{
			mShowWireframe = !mShowWireframe;
			UpdateTechnique();
		}

		if (mDistanceModeEnabled)
		{
			if (mKeyboard->WasKeyPressedThisFrame(DIK_I))
			{
				int tessellationFactor = mMaxTessellationFactor + 1;
				mMaxTessellationFactor = min(tessellationFactor, 64);
			}

			if (mKeyboard->WasKeyPressedThisFrame(DIK_J))
			{
				int tessellationFactor = mMaxTessellationFactor - 1;
				mMaxTessellationFactor = max(tessellationFactor, 1);
			}

			if (mKeyboard->WasKeyPressedThisFrame(DIK_O))
			{
				float tesselletionDistance = mMinTessellationDistance + 1;
				mMinTessellationDistance = min(tesselletionDistance, mMaxTessellationDistance);
			}

			if (mKeyboard->WasKeyPressedThisFrame(DIK_K))
			{
				float tesselletionDistance = mMinTessellationDistance - 1;
				mMinTessellationDistance = max(tesselletionDistance, 1);
			}

			if (mKeyboard->WasKeyPressedThisFrame(DIK_P))
			{
				mMaxTessellationDistance++;
			}

			if (mKeyboard->WasKeyPressedThisFrame(DIK_L))
			{
				float tesselletionDistance = mMaxTessellationDistance - 1;
				mMaxTessellationDistance = max(tesselletionDistance, mMinTessellationDistance);
			}
		}
		else
		{
			if (mKeyboard->WasKeyPressedThisFrame(DIK_UPARROW))
			{
				float tesselletionFactor = mTessellationEdgeFactors[0] + 1;
				tesselletionFactor = min(tesselletionFactor, mMaxTessellationFactor);

				for (UINT i = 0; i < mTessellationEdgeFactors.size(); i++)
				{
					mTessellationEdgeFactors[i] = tesselletionFactor;
				}

				mTessellationInsideFactor = tesselletionFactor;
			}

			if (mKeyboard->WasKeyPressedThisFrame(DIK_DOWNARROW))
			{
				float tesselletionFactor = mTessellationEdgeFactors[0] - 1;
				tesselletionFactor = max(tesselletionFactor, 1);

				for (UINT i = 0; i < mTessellationEdgeFactors.size(); i++)
				{
					mTessellationEdgeFactors[i] = tesselletionFactor;
				}

				mTessellationInsideFactor = tesselletionFactor;
			}

			if (mKeyboard->WasKeyPressedThisFrame(DIK_PGUP))
			{
				float tesselletionFactor = mTessellationEdgeFactors[0] + 5;
				tesselletionFactor = min(tesselletionFactor, mMaxTessellationFactor);

				for (UINT i = 0; i < mTessellationEdgeFactors.size(); i++)
				{
					mTessellationEdgeFactors[i] = tesselletionFactor;
				}

				mTessellationInsideFactor = tesselletionFactor;
			}

			if (mKeyboard->WasKeyPressedThisFrame(DIK_PGDN))
			{
				float tesselletionFactor = mTessellationEdgeFactors[0] - 5;
				tesselletionFactor = max(tesselletionFactor, 1);

				for (UINT i = 0; i < mTessellationEdgeFactors.size(); i++)
				{
					mTessellationEdgeFactors[i] = tesselletionFactor;
				}

				mTessellationInsideFactor = tesselletionFactor;
			}
		}

		if (mKeyboard->WasKeyPressedThisFrame(DIK_G))
		{
			for (GameComponent* component : mGame->Components())
			{
				Grid* grid = component->As<Grid>();
				if (grid != nullptr)
				{
					grid->SetVisible(!grid->Visible());
				}
			}
		}
	}

	void ModelTessellationDemo::UpdateTechnique()
	{
		Technique* technique;

		if (mShowWireframe)
		{
			if (mDistanceModeEnabled)
			{
				technique = mEffect->TechniquesByName().at("solid_color_distance_factors");
			}
			else
			{
				technique = mEffect->TechniquesByName().at("solid_color_manual_factors");
			}
		}
		else
		{
			technique = mEffect->TechniquesByName().at("textured_manual_factors");
		}

		mMaterial->SetCurrentTechnique(*technique);
		mPass = technique->Passes().at(0);
		mInputLayout = mMaterial->InputLayouts().at(mPass);
	}

	void ModelTessellationDemo::Draw(const GameTime& gameTime)
	{
		mRenderStateHelper.SaveRasterizerState();

		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();

		if (mShowWireframe)
		{
			direct3DDeviceContext->RSSetState(RasterizerStates::Wireframe);
		}

		direct3DDeviceContext->IASetInputLayout(mInputLayout);
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

		UINT stride = mMaterial->VertexSize();
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mWorldMatrix);
		mMaterial->WorldViewProjection() << world * mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		mMaterial->World() << world;
		mMaterial->CameraPosition() << mCamera->PositionVector();
		mMaterial->TessellationEdgeFactors() << mTessellationEdgeFactors;
		mMaterial->TessellationInsideFactor() << mTessellationInsideFactor;
		mMaterial->MaxTessellationFactor() << mMaxTessellationFactor;
		mMaterial->MinTessellationDistance() << mMinTessellationDistance;
		mMaterial->MaxTessellationDistance() << mMaxTessellationDistance;
		mMaterial->ColorTexture() << mColorTexture;
		mPass->Apply(0, direct3DDeviceContext);

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);

		direct3DDeviceContext->HSSetShader(nullptr, nullptr, 0);
		direct3DDeviceContext->DSSetShader(nullptr, nullptr, 0);

		mRenderStateHelper.RestoreAll();

		mRenderStateHelper.SaveAll();
		mSpriteBatch->Begin();

		std::wostringstream helpLabel;

		helpLabel << "\nShow Wireframe (Enter): " << (mShowWireframe ? "True" : "False")
			<< "\nTessellation Mode: (Space): " << (mDistanceModeEnabled ? "Distance" : "Manual");

		if (mDistanceModeEnabled)
		{
			helpLabel << "\nMax Tessellation Factor (+I/-J): " << mMaxTessellationFactor
				<< "\nMin Tessellation Distance (+O/-K): " << mMinTessellationDistance
				<< "\nMax Tessellation Distance (+P/-L): " << mMaxTessellationDistance;
		}
		else
		{
			helpLabel << std::setprecision(2) << "\nTessellation Edge Factors: (+Up/-Down +PgUp/-PgDn)[" << mTessellationEdgeFactors[0] << ", " << mTessellationEdgeFactors[1] << ", " << mTessellationEdgeFactors[2] << "]"
				<< "\nTessellation Inside Factor: [" << mTessellationInsideFactor << "]";
		}

		mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();
		mRenderStateHelper.RestoreAll();
    }
}