#pragma once

#include <stdint.h>

typedef struct ShaderProgram ShaderProgram;
typedef struct Vertex Vertex;
typedef struct VertexBuffer VertexBuffer;

typedef enum VertexBufferType {
    VERTEX_BUFFER_STATIC,
    VERTEX_BUFFER_DYNAMIC,
} VertexBufferType;

VertexBuffer *VertexBuffer_Create(VertexBufferType bufferType, uint32_t maximumVertices);
void VertexBuffer_Destroy(VertexBuffer *vertexBuffer);

void VertexBuffer_SetVertexData(VertexBuffer *vertexBuffer, ShaderProgram *shaderProgram,
    Vertex *vertices, uint32_t vertexCount);

uint32_t VertexBuffer_GetArrayId(VertexBuffer *vertexBuffer);
uint32_t VertexBuffer_GetBufferId(VertexBuffer *vertexBuffer);
