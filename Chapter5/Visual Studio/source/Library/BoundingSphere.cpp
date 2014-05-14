#include "BoundingSphere.h"

namespace Library
{
	BoundingSphere::BoundingSphere()
		: mCenter(0.0f, 0.0f, 0.0f), mRadius(0.0f)
	{
	}

    BoundingSphere::BoundingSphere(FXMVECTOR center, float radius)
		: mCenter(), mRadius(radius)
	{
		XMStoreFloat3(&mCenter, center);
	}

	BoundingSphere::BoundingSphere(const XMFLOAT3& center, float radius)
		: mCenter(center), mRadius(radius)
	{
	}

	XMFLOAT3& BoundingSphere::Center()
    {
        return mCenter;
    }

    float& BoundingSphere::Radius()
    {
        return mRadius;
    }
}
