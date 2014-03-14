#include "MaterialDemo.h"
#include "Game.h"
#include "GameException.h"
#include "MatrixHelper.h"
#include "Camera.h"
#include "Utility.h"
#include "Model.h"
#include "Mesh.h"
#include "BasicMaterial.h"

namespace Rendering
{
    RTTI_DEFINITIONS(MaterialDemo)

    MaterialDemo::MaterialDemo(Game& game, Camera& camera)
        : DrawableGameComponent(game, camera),  
          mBasicMaterial(nullptr), mBasicEffect(nullptr), mWorldMatrix(MatrixHelper::Identity),
          mVertexBuffer(nullptr), mIndexBuffer(nullptr), mIndexCount(0)
    {
    }

    MaterialDemo::~MaterialDemo()
    {
        DeleteObject(mBasicMaterial);
        DeleteObject(mBasicEffect);	
        ReleaseObject(mVertexBuffer);
        ReleaseObject(mIndexBuffer);
    }

    void MaterialDemo::Initialize()
    {
        SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

        // Load the model
        std::unique_ptr<Model> model(new Model(*mGame, "Content\\Models\\Sphere.obj", true));

        // Initialize the material
        mBasicEffect = new Effect(*mGame);
        mBasicEffect->LoadCompiledEffect(L"Content\\Effects\\BasicEffect.cso");
        mBasicMaterial = new BasicMaterial();
        mBasicMaterial->Initialize(mBasicEffect);
        
        // Create the vertex and index buffers
        Mesh* mesh = model->Meshes().at(0);
        mBasicMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), *mesh, &mVertexBuffer);
        mesh->CreateIndexBuffer(&mIndexBuffer);
        mIndexCount = mesh->Indices().size();
    }

    void MaterialDemo::Draw(const GameTime& gameTime)
    {
        ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();        
        direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        Pass* pass = mBasicMaterial->CurrentTechnique()->Passes().at(0);
        ID3D11InputLayout* inputLayout = mBasicMaterial->InputLayouts().at(pass);
        direct3DDeviceContext->IASetInputLayout(inputLayout);

        UINT stride = mBasicMaterial->VertexSize();
        UINT offset = 0;
        direct3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);		
        direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
                
        XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
        XMMATRIX wvp = worldMatrix * mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
        mBasicMaterial->WorldViewProjection() << wvp;

        pass->Apply(0, direct3DDeviceContext);

        direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
    }
}