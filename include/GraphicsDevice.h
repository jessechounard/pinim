#pragma once

#include <stdint.h>

#include "Types.h"

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

void GraphicsDevice_BindRenderTarget(
    GraphicsDevice *graphicsDevice, Texture *renderTarget, bool setViewport);
void GraphicsDevice_UnbindRenderTarget(GraphicsDevice *graphicsDevice, bool resetViewport);

bool GraphicsDevice_IsUsingRenderTarget(GraphicsDevice *device);

void GraphicsDevice_ReadPixels(GraphicsDevice *graphicsDevice, uint32_t x, uint32_t y,
    uint32_t width, uint32_t height, uint8_t *pixels);

void GraphicsDevice_ApplyShaderProgram(
    GraphicsDevice *graphicsDevice, ShaderProgram *shaderProgram);

void GraphicsDevice_BeginFrame(GraphicsDevice *graphicsDevice);
void GraphicsDevice_EndFrame(GraphicsDevice *graphicsDevice);

void GraphicsDevice_DrawPrimitives(GraphicsDevice *graphicsDevice, VertexBuffer *vertexBuffer,
    RenderPrimitiveType primitiveType, uint32_t vertexStart, uint32_t primitiveCount);
