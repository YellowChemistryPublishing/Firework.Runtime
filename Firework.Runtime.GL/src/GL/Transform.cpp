#include "Transform.h"

#include <bx/math.h>

using namespace Firework;
using namespace Firework::GL;

void RenderTransform::translate(sysm::vector3 vec)
{
    this->tf = sysm::matrix4x4::translate(vec) * this->tf;
}
void RenderTransform::rotate(sysm::quaternion rot)
{
    this->tf = sysm::matrix4x4::rotate(rot) * this->tf;
}
void RenderTransform::scale(sysm::vector3 vec)
{
    this->tf = sysm::matrix4x4::scale(vec) * this->tf;
}

void RenderTransform::regenerateNormalMatrix()
{
    float m1[3][3], m2[3][3]
    {
        { this->tf[{ 0, 0 }], this->tf[{ 0, 1 }], this->tf[{ 0, 2 }] },
        { this->tf[{ 1, 0 }], this->tf[{ 1, 1 }], this->tf[{ 1, 2 }] },
        { this->tf[{ 2, 0 }], this->tf[{ 2, 1 }], this->tf[{ 2, 2 }] }
    };
    bx::mtx3Inverse(m1[0], m2[0]);
    for (size_t i = 0; i < 3; i++)
        for (size_t j = 0; j < 3; j++)
            this->normalMatrix[i][j] = m1[i][j];
}
