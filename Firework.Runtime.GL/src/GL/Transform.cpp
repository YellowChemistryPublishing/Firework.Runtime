#include "Transform.h"

#include <bx/math.h>

using namespace Firework;
using namespace Firework::Mathematics;
using namespace Firework::GL;

void RenderTransform::translate(Mathematics::Vector3 vec)
{
    this->tf = Matrix4x4::translate(vec) * this->tf;
}
void RenderTransform::rotate(Mathematics::Vector3 vec)
{
    this->tf = Matrix4x4::rotate(vec) * this->tf;
}
void RenderTransform::rotate(Mathematics::Quaternion rot)
{
    this->tf = Matrix4x4::rotate(rot) * this->tf;
}
void RenderTransform::scale(Mathematics::Vector3 vec)
{
    this->tf = Matrix4x4::scale(vec) * this->tf;
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
