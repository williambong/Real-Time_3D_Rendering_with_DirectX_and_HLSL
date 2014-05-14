#pragma once

#include "Common.h"

namespace Library
{
    class BoundingSphere
    {
    public:
		BoundingSphere();
		BoundingSphere(FXMVECTOR center, float radius);
		BoundingSphere(const XMFLOAT3& center, float radius);
		
		XMFLOAT3& Center();
        float& Radius();
    
	private:
		XMFLOAT3 mCenter;
		float mRadius;
    };
}

