#pragma once

#include "Firework.Runtime.RenderPipeline.Exports.h"

#include <GL/ModelLoader.h>

namespace Firework
{
    namespace GL
    {
        class __firework_rp_api RenderPipeline final
        {
        public:
            RenderPipeline() = delete;

            static void init();
            static void deinit();

            static void drawMesh(GL::StaticMeshHandle mesh);
        };
    }
}