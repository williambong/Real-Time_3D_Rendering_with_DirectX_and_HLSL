#pragma once

#include "Common.h"
#include "RenderTarget.h"

namespace Library
{
    class Game;

    class FullScreenRenderTarget : public RenderTarget
    {
		RTTI_DECLARATIONS(FullScreenRenderTarget, RenderTarget)

    public:
        FullScreenRenderTarget(Game& game);
        ~FullScreenRenderTarget();

        ID3D11ShaderResourceView* OutputTexture() const;
        ID3D11RenderTargetView* RenderTargetView() const;
        ID3D11DepthStencilView* DepthStencilView() const;

        virtual void Begin() override;
		virtual void End() override;

    private:
        FullScreenRenderTarget();
        FullScreenRenderTarget(const FullScreenRenderTarget& rhs);
        FullScreenRenderTarget& operator=(const FullScreenRenderTarget& rhs);

        Game* mGame;
        ID3D11RenderTargetView* mRenderTargetView;
        ID3D11DepthStencilView* mDepthStencilView;
        ID3D11ShaderResourceView* mOutputTexture;
    };
}