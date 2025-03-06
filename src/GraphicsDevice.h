#pragma once

#include <stdint.h>

typedef enum GraphicsAPI {
    GRAPHICS_API_OPENGL,
} GraphicsAPI;

typedef enum VerticalSyncType {
    VERTICAL_SYNC_ADAPTIVE,
    VERTICAL_SYNC_ENABLED,
    VERTICAL_SYNC_DISABLED,
} VerticalSyncType;

typedef enum RenderPrimitiveType {
    RENDER_PRIMITIVE_TRAINGLES,
    RENDER_PRIMITIVE_TRIANGLE_STRIP,
    RENDER_PRIMITIVE_LINES,
    RENDER_PRIMITIVE_LINE_STRIP,
    RENDER_PRIMITIVE_POINTS,
} RenderPrimitiveType;

typedef enum BlendMode {
    BLEND_MODE_INVALID = -1,
    BLEND_MODE_NONE,
    BLEND_MODE_ADDITIVE,
    BLEND_MODE_ALPHA,
    BLEND_MODE_PREMULTIPLIED_ALPHA,
} BlendMode;

typedef struct Rectangle {
    int x, y, width, height;
} Rectangle;

typedef struct Color {
    float r, g, b, a;
} Color;

typedef struct Vertex {
    float x, y;
    float u, v;
    float r, g, b, a;
} Vertex;

typedef struct GraphicsDevice GraphicsDevice;
typedef struct ShaderProgram ShaderProgram;
typedef struct VertexBuffer VertexBuffer;

uint32_t GraphicsDevice_PrepareSDLWindowAttributes(GraphicsAPI api);

GraphicsDevice *GraphicsDevice_Create(
    GraphicsAPI api, SDL_Window *window, VerticalSyncType vsyncType);
void GraphicsDevice_Destroy(GraphicsDevice *device);

void GraphicsDevice_SetViewport(GraphicsDevice *device, Rectangle *viewport);
void GraphicsDevice_GetViewport(GraphicsDevice *device, Rectangle *viewport);

uint32_t GraphicsDevice_GetWindowWidth(GraphicsDevice *device);
uint32_t GraphicsDevice_GetWindowHeight(GraphicsDevice *device);

void GraphicsDevice_ClearScreen(GraphicsDevice *device, Color *color);

void GraphicsDevice_SetBlendMode(GraphicsDevice *device, BlendMode blendMode);

void GraphicsDevice_EnableScissorsRectangle(GraphicsDevice *device, Rectangle *scissorsRectangle);
void GraphicsDevice_DisableScissorsRectangle(GraphicsDevice *device);

// void GraphicsDevice_BindTexture(GraphicsDevice *device, const Texture &texture);

// void GraphicsDevice_BindRenderTarget(GraphicsDevice *device, const Texture &texture, bool
// setViewport = true); void GraphicsDevice_UnbindRenderTarget(GraphicsDevice *device, bool
// resetViewport = true);

bool GraphicsDevice_IsUsingRenderTarget(GraphicsDevice *device);

void GraphicsDevice_ReadPixels(GraphicsDevice *graphicsDevice, uint32_t x, uint32_t y,
    uint32_t width, uint32_t height, uint8_t *pixels);

void GraphicsDevice_ApplyShaderProgram(
    GraphicsDevice *graphicsDevice, ShaderProgram *shaderProgram);

void GraphicsDevice_BeginFrame(GraphicsDevice *graphicsDevice);
void GraphicsDevice_EndFrame(GraphicsDevice *graphicsDevice);

void GraphicsDevice_DrawPrimitives(GraphicsDevice *graphicsDevice, VertexBuffer *vertexBuffer,
    RenderPrimitiveType primitiveType, uint32_t vertexStart, uint32_t primitiveCount);
