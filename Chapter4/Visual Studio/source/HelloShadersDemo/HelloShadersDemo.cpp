#include "HelloShadersDemo.h"
#include "Game.h"
#include "GameException.h"
#include "MatrixHelper.h"
#include "Camera.h"
#include "Utility.h"
#include "Model.h"
#include "Mesh.h"
#include "HelloShadersMaterial.h"
#include "RenderStateHelper.h"
#include <sstream>
#include <SpriteBatch.h>
#include <SpriteFont.h>

namespace Rendering
{
    RTTI_DEFINITIONS(HelloShadersDemo)

    HelloShadersDemo::HelloShadersDemo(Game& game, Camera& camera)
        : DrawableGameComponent(game, camera),  
          mMaterial(nullptr), mEffect(nullptr), mWorldMatrix(MatrixHelper::Identity),
          mVertexBuffer(nullptr), mIndexBuffer(nullptr), mIndexCount(0),
		  mRenderStateHelper(nullptr), mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f)
    {
    }

    HelloShadersDemo::~HelloShadersDemo()
    {
		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);
		DeleteObject(mRenderStateHelper);
        DeleteObject(mMaterial);
        DeleteObject(mEffect);	
        ReleaseObject(mVertexBuffer);
        ReleaseObject(mIndexBuffer);
    }

    void HelloShadersDemo::Initialize()
    {
        SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

        // Load the model
        std::unique_ptr<Model> model(new Model(*mGame, "Content\\Models\\Sphere.obj", true));

        // Initialize the material
        mEffect = new Effect(*mGame);
		mEffect->CompileFromFile(L"Content\\Effects\\HelloShaders.fx");

        mMaterial = new HelloShadersMaterial();
        mMaterial->Initialize(*mEffect);
        
        // Create the vertex and index buffers
        Mesh* mesh = model->Meshes().at(0);
        mMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), *mesh, &mVertexBuffer);
        mesh->CreateIndexBuffer(&mIndexBuffer);
        mIndexCount = mesh->Indices().size();

		mRenderStateHelper = new RenderStateHelper(*mGame);

		mSpriteBatch = new SpriteBatch(mGame->Direct3DDeviceContext());
		mSpriteFont = new SpriteFont(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");
    }

    void HelloShadersDemo::Draw(const GameTime& gameTime)
    {
        ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();        
        direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        Pass* pass = mMaterial->CurrentTechnique()->Passes().at(0);
        ID3D11InputLayout* inputLayout = mMaterial->InputLayouts().at(pass);
        direct3DDeviceContext->IASetInputLayout(inputLayout);

        UINT stride = mMaterial->VertexSize();
        UINT offset = 0;
        direct3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);		
        direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
                
        XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
        XMMATRIX wvp = worldMatrix * mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
        mMaterial->WorldViewProjection() << wvp;

        pass->Apply(0, direct3DDeviceContext);

        direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);

		mRenderStateHelper->SaveAll();
		mSpriteBatch->Begin();

		std::wostringstream helpLabel;
		helpLabel << L"Move Camera (W, A, S, D + Mouse)\n";

		mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();
		mRenderStateHelper->RestoreAll();
    }
}