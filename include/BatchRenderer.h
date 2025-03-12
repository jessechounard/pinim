#pragma once

#include <stdint.h>

#include "GameMath.h"
#include "Types.h"

BatchRenderer *BatchRenderer_Create(GraphicsDevice *graphicsDevice, uint32_t maximumTriangles);
void BatchRenderer_Destroy(BatchRenderer *batchRenderer);

// texture can be null if your shader doesn't use it
// shaderProgram can be null if you want to use the default shaders
// texture and shaderProgram cannot both be null
void BatchRenderer_Begin(BatchRenderer *batchRenderer, BlendMode blendMode, Texture *texture,
    ShaderProgram *shaderProgram, Matrix4 transformMatrix);
void BatchRenderer_End(BatchRenderer *batchRenderer);
// push through all batched polys without ending the batch
void BatchRenderer_Flush(BatchRenderer *batchRenderer);

bool BatchRenderer_BatchActive(BatchRenderer *batchRenderer);

// use a source rectangle to calculate UVs for the vertices
// color can be null if you want to use White
void BatchRenderer_BatchQuad(BatchRenderer *batchRenderer, Rectangle *sourceRectangle,
    Vector2 position, float rotation, Vector2 scale, Vector2 origin, UVMode uvMode, Color *color);

void BatchRenderer_BatchQuadUV(
    BatchRenderer *batchRenderer, Vector2 uv0, Vector2 uv1, Vector2 xy0, Vector2 xy1, Color *color);

// render raw vertices as triangles. (Three vertices per triangle.)
void BatchRenderer_BatchTriangles(
    BatchRenderer *batchRenderer, Vertex2d *triangleVertices, int triangleCount);
