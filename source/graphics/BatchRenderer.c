#include <assert.h>
#include <SDL3/SDL.h>

#include <BatchRenderer.h>
#define PINIM_GAME_MATH_IMPLEMENTATION
#include <GameMath.h>
#include <GraphicsDevice.h>
#include <ShaderProgram.h>
#include <Texture.h>
#include <VertexBuffer.h>

void CreateOrthographicOffCenterMatrix(float left, float right, float bottom, float top,
    float zNearPlane, float zFarPlane, float matrix[16]) {
    float result[16] = {2.0f / (right - left),
        0,
        0,
        0,
        0,
        2.0f / (top - bottom),
        0,
        0,
        0,
        0,
        -2.0f / (zFarPlane - zNearPlane),
        0,
        -(right + left) / (right - left),
        -(top + bottom) / (top - bottom),
        -(zFarPlane - zNearPlane) / (zFarPlane - zNearPlane),
        1};

    SDL_memcpy(matrix, result, sizeof(float) * 16);
}

struct BatchRenderer {
    GraphicsDevice *graphicsDevice;
    ShaderProgram *defaultShaderProgram;
    ShaderProgram *currentShaderProgram;
    VertexBuffer *vertexBuffer;
    Texture *texture;
    Matrix4 transformMatrix;
    BlendMode blendMode;
    uint32_t activeVertices;
    uint32_t maximumVertices;
    Vertex2d *vertices;
    bool batchStarted;
};

char defaultVertexShaderSource[] =
    // input from CPU
    "#version 410\n"
    "in vec4 position;\n"
    "in vec4 color;\n"
    "in vec2 texcoord;\n"
    // output to fragment shader
    "out vec4 v_color;\n"
    "out vec2 v_texcoord;\n"
    // custom input from program
    "uniform mat4 ProjectionMatrix;\n"
    //
    "void main()\n"
    "{\n"
    "	gl_Position = ProjectionMatrix * position;\n"
    "	v_color = color;\n"
    "	v_texcoord = texcoord;\n"
    "}\n";

char defaultFragmentShaderSource[] =
    // input from vertex shader
    "#version 410\n"
    "in vec4 v_color;\n"
    "in vec2 v_texcoord;\n"
    "out vec4 fragColor;\n"
    // custom input from program
    "uniform sampler2D TextureSampler;\n"
    //
    "void main()\n"
    "{\n"
    "	fragColor = texture2D(TextureSampler, v_texcoord) * v_color;\n"
    "}\n";

BatchRenderer *BatchRenderer_Create(GraphicsDevice *graphicsDevice, uint32_t maximumTriangles) {
    assert(graphicsDevice != NULL);
    assert(maximumTriangles > 0);

    BatchRenderer *batchRenderer;
    batchRenderer = SDL_calloc(1, sizeof(BatchRenderer));
    if (batchRenderer == NULL) {
        SDL_Log("SDL_calloc failed");
        return NULL;
    }
    batchRenderer->maximumVertices = maximumTriangles * 3;

    VertexShader *vertexShader = VertexShader_CreateFromBuffer(
        graphicsDevice, defaultVertexShaderSource, sizeof(defaultVertexShaderSource));
    if (vertexShader == NULL) {
        SDL_Log("VertexShader_CreateFromBuffer failed");
        SDL_free(batchRenderer);
        return NULL;
    }

    FragmentShader *fragmentShader = FragmentShader_CreateFromBuffer(
        graphicsDevice, defaultFragmentShaderSource, sizeof(defaultFragmentShaderSource));
    if (fragmentShader == NULL) {
        SDL_Log("FragmentShader_CreateFromBuffer failed");
        VertexShader_Destroy(vertexShader);
        SDL_free(batchRenderer);
        return NULL;
    }

    batchRenderer->defaultShaderProgram =
        ShaderProgram_Create(graphicsDevice, vertexShader, fragmentShader);
    if (batchRenderer->defaultShaderProgram == NULL) {
        SDL_Log("ShaderProgram_Create failed");
        FragmentShader_Destroy(fragmentShader);
        VertexShader_Destroy(vertexShader);
        SDL_free(batchRenderer);
        return NULL;
    }

    FragmentShader_Destroy(fragmentShader);
    VertexShader_Destroy(vertexShader);

    batchRenderer->vertexBuffer =
        VertexBuffer_Create(VERTEX_BUFFER_DYNAMIC, batchRenderer->maximumVertices);
    if (batchRenderer->vertexBuffer == NULL) {
        SDL_Log("VertexBuffer_Create failed");
        ShaderProgram_Destroy(batchRenderer->defaultShaderProgram);
        SDL_free(batchRenderer);
    }

    batchRenderer->vertices = SDL_calloc(batchRenderer->maximumVertices, sizeof(Vertex2d));
    if (batchRenderer->vertices == NULL) {
        SDL_Log("SDL_calloc failed");
        VertexBuffer_Destroy(batchRenderer->vertexBuffer);
        ShaderProgram_Destroy(batchRenderer->defaultShaderProgram);
        SDL_free(batchRenderer);
    }

    batchRenderer->graphicsDevice = graphicsDevice;

    return batchRenderer;
}

void BatchRenderer_Destroy(BatchRenderer *batchRenderer) {
    assert(batchRenderer != NULL);

    SDL_free(batchRenderer->vertices);
    VertexBuffer_Destroy(batchRenderer->vertexBuffer);
    ShaderProgram_Destroy(batchRenderer->defaultShaderProgram);
    SDL_free(batchRenderer);
}

void BatchRenderer_Begin(BatchRenderer *batchRenderer, BlendMode blendMode, Texture *texture,
    ShaderProgram *shaderProgram, Matrix4 transformMatrix) {
    assert(batchRenderer != NULL);

    if (batchRenderer->batchStarted) {
        SDL_Log("BatchRenderer_Begin called on already started BatchRenderer");
        return;
    }

    batchRenderer->activeVertices = 0;
    batchRenderer->batchStarted = true;
    batchRenderer->blendMode = blendMode;
    batchRenderer->texture = texture;
    batchRenderer->currentShaderProgram =
        (shaderProgram != NULL) ? shaderProgram : batchRenderer->defaultShaderProgram;

    Matrix4_Copy(transformMatrix, batchRenderer->transformMatrix);
}

void BatchRenderer_End(BatchRenderer *batchRenderer) {
    assert(batchRenderer != NULL);

    if (!batchRenderer->batchStarted) {
        SDL_Log("BatchRenderer_End called on a not started BatchRenderer");
        return;
    }

    BatchRenderer_Flush(batchRenderer);

    batchRenderer->batchStarted = false;
}

void BatchRenderer_Flush(BatchRenderer *batchRenderer) {
    assert(batchRenderer != NULL);

    if (!batchRenderer->batchStarted || batchRenderer->activeVertices < 3) {
        return;
    }

    Matrix4 projectionMatrix;
    Rectangle viewport;

    GraphicsDevice_GetViewport(batchRenderer->graphicsDevice, &viewport);

    if (GraphicsDevice_IsUsingRenderTarget(batchRenderer->graphicsDevice)) {
        Matrix4_OrthoCamera(viewport.x,
            (viewport.x + viewport.width),
            viewport.y,
            (viewport.y + viewport.height),
            -1,
            1000,
            projectionMatrix);
    } else {
        Matrix4_OrthoCamera(viewport.x,
            (viewport.x + viewport.width),
            (viewport.y + viewport.height),
            viewport.y,
            -1,
            1000,
            projectionMatrix);
    }

    Matrix4_Multiply(projectionMatrix, batchRenderer->transformMatrix, projectionMatrix);

    GraphicsDevice_SetBlendMode(batchRenderer->graphicsDevice, batchRenderer->blendMode);
    GraphicsDevice_ApplyShaderProgram(
        batchRenderer->graphicsDevice, batchRenderer->currentShaderProgram);

    int32_t textureSamplerLocation =
        ShaderProgram_GetParameterLocation(batchRenderer->currentShaderProgram, "TextureSampler");
    if (textureSamplerLocation != -1 && batchRenderer->texture != NULL) {
        ShaderProgram_SetParameterTexture2D(
            batchRenderer->currentShaderProgram, "TextureSampler", batchRenderer->texture, 0);
    }

    int32_t projectionMatrixLocation =
        ShaderProgram_GetParameterLocation(batchRenderer->currentShaderProgram, "ProjectionMatrix");
    if (projectionMatrixLocation != -1) {
        ShaderProgram_SetParameterMatrix4(
            batchRenderer->currentShaderProgram, "ProjectionMatrix", projectionMatrix);
    }

    ShaderProgram_ApplyParameters(batchRenderer->currentShaderProgram);

    VertexBuffer_SetVertexData(batchRenderer->vertexBuffer,
        batchRenderer->currentShaderProgram,
        batchRenderer->vertices,
        batchRenderer->activeVertices);

    GraphicsDevice_DrawPrimitives(batchRenderer->graphicsDevice,
        batchRenderer->vertexBuffer,
        RENDER_PRIMITIVE_TRIANGLES,
        0,
        batchRenderer->activeVertices / 3);

    batchRenderer->activeVertices = 0;
}

bool BatchRenderer_BatchActive(BatchRenderer *batchRenderer) {
    assert(batchRenderer != NULL);
    return batchRenderer->batchStarted;
}

void BatchRenderer_BatchQuad(BatchRenderer *batchRenderer, Rectangle *sourceRectangle,
    Vector2 position, float rotation, Vector2 scale, Vector2 origin, UVMode uvMode, Color *color) {
    assert(batchRenderer != NULL);

    if (!batchRenderer->batchStarted) {
        SDL_Log("BatchRenderer_BatchQuad called on unstarted batch");
        return;
    }

    if (batchRenderer->activeVertices + 6 > batchRenderer->maximumVertices) {
        BatchRenderer_Flush(batchRenderer);
    }

    float destX = position[0];
    float destY = position[1];
    float destW = scale[0];
    float destH = scale[1];
    int textureW = Texture_GetWidth(batchRenderer->texture);
    int textureH = Texture_GetHeight(batchRenderer->texture);
    Rectangle source;

    if (sourceRectangle != NULL) {
        source = *sourceRectangle;
    } else {
        source = (Rectangle){.x = 0, .y = 0, .width = textureW, .height = textureH};
    }

    if (sourceRectangle != NULL) {
        destW *= sourceRectangle->width;
        destH *= sourceRectangle->height;
    } else {
        destW *= textureW;
        destH *= textureH;
    }

    Vector2 uvs[4];
    if ((uvMode & UVMODE_ROTATED_CW90) != 0) {
        uvs[0][0] = (source.x + source.height) / (float)textureW;
        uvs[0][1] = source.y / (float)textureH;
        uvs[1][0] = (source.x + source.height) / (float)textureW;
        uvs[1][1] = (source.y + source.width) / (float)textureH;
        uvs[2][0] = source.x / (float)textureW;
        uvs[2][1] = (source.y + source.width) / (float)textureH;
        uvs[3][0] = source.x / (float)textureW;
        uvs[3][1] = source.y / (float)textureH;
    } else {
        uvs[0][0] = source.x / (float)textureW;
        uvs[0][1] = source.y / (float)textureH;
        uvs[1][0] = (source.x + source.width) / (float)textureW;
        uvs[1][1] = source.y / (float)textureH;
        uvs[2][0] = (source.x + source.width) / (float)textureW;
        uvs[2][1] = (source.y + source.height) / (float)textureH;
        uvs[3][0] = source.x / (float)textureW;
        uvs[3][1] = (source.y + source.height) / (float)textureH;
    }

    if ((uvMode & UVMODE_FLIP_HORIZONTAL) != 0) {
        Vector2 t;
        Vector2_Copy(uvs[0], t);
        Vector2_Copy(uvs[1], uvs[0]);
        Vector2_Copy(t, uvs[1]);

        Vector2_Copy(uvs[2], t);
        Vector2_Copy(uvs[3], uvs[2]);
        Vector2_Copy(t, uvs[3]);
    }

    if ((uvMode & UVMODE_FLIP_VERTICAL) != 0) {
        Vector2 t;
        Vector2_Copy(uvs[0], t);
        Vector2_Copy(uvs[3], uvs[0]);
        Vector2_Copy(t, uvs[3]);

        Vector2_Copy(uvs[1], t);
        Vector2_Copy(uvs[2], uvs[1]);
        Vector2_Copy(t, uvs[2]);
    }

    float rotationSin = SDL_sin(rotation);
    float rotationCos = SDL_cos(rotation);
    Color c;
    if (color != NULL) {
        c.r = color->r;
        c.g = color->g;
        c.b = color->b;
        c.a = color->a;
    } else {
        c.r = 1;
        c.g = 1;
        c.b = 1;
        c.a = 1;
    }

    float cornerX = -origin[0] * destW;
    float cornerY = -origin[1] * destH;
    Vertex2d *vertices = &batchRenderer->vertices[0] + batchRenderer->activeVertices;
    vertices->x = cornerX * rotationCos - cornerY * rotationSin + destX;
    vertices->y = cornerX * rotationSin + cornerY * rotationCos + destY;
    vertices->u = uvs[0][0];
    vertices->v = uvs[0][1];
    vertices->r = c.r;
    vertices->g = c.g;
    vertices->b = c.b;
    vertices->a = c.a;
    vertices++;

    cornerX = (1.0f - origin[0]) * destW;
    cornerY = -origin[1] * destH;
    vertices->x = cornerX * rotationCos - cornerY * rotationSin + destX;
    vertices->y = cornerX * rotationSin + cornerY * rotationCos + destY;
    vertices->u = uvs[1][0];
    vertices->v = uvs[1][1];
    vertices->r = c.r;
    vertices->g = c.g;
    vertices->b = c.b;
    vertices->a = c.a;
    vertices++;

    cornerX = (1.0f - origin[0]) * destW;
    cornerY = (1.0f - origin[1]) * destH;
    vertices->x = cornerX * rotationCos - cornerY * rotationSin + destX;
    vertices->y = cornerX * rotationSin + cornerY * rotationCos + destY;
    vertices->u = uvs[2][0];
    vertices->v = uvs[2][1];
    vertices->r = c.r;
    vertices->g = c.g;
    vertices->b = c.b;
    vertices->a = c.a;
    vertices++;

    *vertices = *(vertices - 3);
    vertices++;
    *vertices = *(vertices - 2);
    vertices++;

    cornerX = -origin[0] * destW;
    cornerY = (1.0f - origin[1]) * destH;
    vertices->x = cornerX * rotationCos - cornerY * rotationSin + destX;
    vertices->y = cornerX * rotationSin + cornerY * rotationCos + destY;
    vertices->u = uvs[3][0];
    vertices->v = uvs[3][1];
    vertices->r = c.r;
    vertices->g = c.g;
    vertices->b = c.b;
    vertices->a = c.a;

    batchRenderer->activeVertices += 6;
}

void BatchRenderer_BatchQuadUV(BatchRenderer *batchRenderer, Vector2 uv0, Vector2 uv1, Vector2 xy0,
    Vector2 xy1, Color *color) {
    assert(batchRenderer != NULL);

    if (!batchRenderer->batchStarted) {
        SDL_Log("BatchRenderer_BatchQuadUV called on unstarted batch");
        return;
    }

    Color c;
    if (color != NULL) {
        c.r = color->r;
        c.g = color->g;
        c.b = color->b;
        c.a = color->a;
    } else {
        c.r = 1;
        c.g = 1;
        c.b = 1;
        c.a = 1;
    }

    if (batchRenderer->activeVertices + 6 > batchRenderer->maximumVertices) {
        BatchRenderer_Flush(batchRenderer);
    }

    Vector2 uvs[4];
    uvs[0][0] = uv0[0];
    uvs[0][1] = uv0[1];
    uvs[1][0] = uv1[0];
    uvs[1][1] = uv0[1];
    uvs[2][0] = uv1[0];
    uvs[2][1] = uv1[1];
    uvs[3][0] = uv0[0];
    uvs[3][1] = uv1[1];

    Vertex2d *vertices = &batchRenderer->vertices[0] + batchRenderer->activeVertices;
    vertices->x = xy0[0];
    vertices->y = xy0[1];
    vertices->u = uvs[0][0];
    vertices->v = uvs[0][1];
    vertices->r = c.r;
    vertices->g = c.g;
    vertices->b = c.b;
    vertices->a = c.a;
    vertices++;

    vertices->x = xy1[0];
    vertices->y = xy0[1];
    vertices->u = uvs[1][0];
    vertices->v = uvs[1][1];
    vertices->r = c.r;
    vertices->g = c.g;
    vertices->b = c.b;
    vertices->a = c.a;
    vertices++;

    vertices->x = xy1[0];
    vertices->y = xy1[1];
    vertices->u = uvs[2][0];
    vertices->v = uvs[2][1];
    vertices->r = c.r;
    vertices->g = c.g;
    vertices->b = c.b;
    vertices->a = c.a;
    vertices++;

    *vertices = *(vertices - 3);
    vertices++;
    *vertices = *(vertices - 2);
    vertices++;

    vertices->x = xy0[0];
    vertices->y = xy1[1];
    vertices->u = uvs[3][0];
    vertices->v = uvs[3][1];
    vertices->r = c.r;
    vertices->g = c.g;
    vertices->b = c.b;
    vertices->a = c.a;

    batchRenderer->activeVertices += 6;
}

void BatchRenderer_BatchTriangles(
    BatchRenderer *batchRenderer, Vertex2d *triangleVertices, int triangleCount) {
    assert(batchRenderer != NULL);
    assert(triangleVertices != NULL);
    assert(triangleCount > 0);

    if (!batchRenderer->batchStarted) {
        SDL_Log("BatchRenderer_BatchTriangles called on unstarted batch");
        return;
    }

    Vertex2d *currentTriangleVertex = triangleVertices;

    for (int index = 0; index < triangleCount * 3; index += 3) {
        if (batchRenderer->activeVertices + 3 > batchRenderer->maximumVertices) {
            BatchRenderer_Flush(batchRenderer);
        }

        Vertex2d *vertex = &batchRenderer->vertices[batchRenderer->activeVertices];
        *vertex++ = *currentTriangleVertex++;
        *vertex++ = *currentTriangleVertex++;
        *vertex++ = *currentTriangleVertex++;
        batchRenderer->activeVertices += 3;
    }
}
