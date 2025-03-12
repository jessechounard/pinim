#include <assert.h>
#include <glad/gl.h>
#include <SDL3/SDL.h>

#include <GraphicsDevice.h>
#include <ShaderProgram.h>
#include <VertexBuffer.h>

struct VertexBuffer {
    uint32_t vertexArrayId;
    uint32_t vertexBufferId;
};

VertexBuffer *VertexBuffer_Create(VertexBufferType bufferType, uint32_t maximumVertices) {
    assert(maximumVertices > 0);

    VertexBuffer *vertexBuffer = SDL_malloc(sizeof(VertexBuffer));
    if (vertexBuffer == NULL) {
        SDL_Log("SDL_malloc failed");
        return NULL;
    }

    glGenVertexArrays(1, &vertexBuffer->vertexArrayId);
    glBindVertexArray(vertexBuffer->vertexArrayId);
    glEnableVertexAttribArray(vertexBuffer->vertexArrayId);
    glGenBuffers(1, &vertexBuffer->vertexBufferId);

    GLenum bufferUsage = (bufferType == VERTEX_BUFFER_STATIC) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, maximumVertices * sizeof(Vertex2d), NULL, bufferUsage);

    return vertexBuffer;
}

void VertexBuffer_Destroy(VertexBuffer *vertexBuffer) {
    glDeleteBuffers(1, &vertexBuffer->vertexBufferId);
    glDeleteVertexArrays(1, &vertexBuffer->vertexArrayId);
    SDL_free(vertexBuffer);
}

void VertexBuffer_SetVertexData(VertexBuffer *vertexBuffer, ShaderProgram *shaderProgram,
    Vertex2d *vertices, uint32_t vertexCount) {
    assert(shaderProgram != NULL);
    assert(vertices != NULL);
    assert(vertexCount > 0);

    glBindVertexArray(vertexBuffer->vertexArrayId);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vertexBufferId);

    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount * sizeof(Vertex2d), vertices);

    int32_t positionLocation = ShaderProgram_GetAttributeLocation(shaderProgram, "position");
    if (positionLocation != -1) {
        glVertexAttribPointer(positionLocation, 2, GL_FLOAT, 0, sizeof(Vertex2d), 0);
        glEnableVertexAttribArray(positionLocation);
    }

    int32_t colorLocation = ShaderProgram_GetAttributeLocation(shaderProgram, "color");
    if (colorLocation != -1) {
        glVertexAttribPointer(
            colorLocation, 4, GL_FLOAT, 0, sizeof(Vertex2d), (void *)(sizeof(float) * 4));
        glEnableVertexAttribArray(colorLocation);
    }

    int32_t texcoordLocation = ShaderProgram_GetAttributeLocation(shaderProgram, "texcoord");
    if (texcoordLocation != -1) {
        glVertexAttribPointer(
            texcoordLocation, 2, GL_FLOAT, 0, sizeof(Vertex2d), (void *)(sizeof(float) * 2));
        glEnableVertexAttribArray(texcoordLocation);
    }

    return;
}

uint32_t VertexBuffer_GetArrayId(VertexBuffer *vertexBuffer) {
    return vertexBuffer->vertexArrayId;
}

uint32_t VertexBuffer_GetBufferId(VertexBuffer *vertexBuffer) {
    return vertexBuffer->vertexBufferId;
}
