#pragma once

#include <stdint.h>

#include "Types.h"

VertexBuffer *VertexBuffer_Create(VertexBufferType bufferType, uint32_t maximumVertices);
void VertexBuffer_Destroy(VertexBuffer *vertexBuffer);

void VertexBuffer_SetVertexData(VertexBuffer *vertexBuffer, ShaderProgram *shaderProgram,
    Vertex2d *vertices, uint32_t vertexCount);

uint32_t VertexBuffer_GetArrayId(VertexBuffer *vertexBuffer);
uint32_t VertexBuffer_GetBufferId(VertexBuffer *vertexBuffer);
