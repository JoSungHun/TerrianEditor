#include "Framework.h"
#include "Math.h"

//floor, ceil, round

const D3DXVECTOR3 Math::Floor(const D3DXVECTOR3 & value)
{
    return D3DXVECTOR3
    (
        floor(value.x),
        floor(value.y),
        floor(value.z)
    );
}
