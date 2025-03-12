#pragma once

#include <stdint.h>

#include "Types.h"

Texture *Texture_Create(GraphicsDevice *graphicsDevice, char *fileName, TextureFilter textureFilter,
    TextureType textureType);
Texture *Texture_CreateFromBuffer(GraphicsDevice *graphicsDevice, void *buffer, uint32_t length,
    TextureFilter textureFilter, TextureType textureType);
Texture *Texture_CreateFromPixelData(GraphicsDevice *graphicsDevice, uint32_t width,
    uint32_t height, uint8_t *pixelData, uint32_t dataLength, TextureFilter textureFilter,
    TextureType textureType);
void Texture_Destroy(Texture *texture);

void Texture_SetTextureData(Texture *texture, uint32_t x, uint32_t y, uint32_t w, uint32_t h,
    uint8_t *pixelData, uint32_t dataLength);

TextureFilter Texture_GetTextureFilter(Texture *texture);

void Texture_SetTextureFilter(Texture *texture, TextureFilter textureFilter);

TextureType Texture_GetTextureType(Texture *texture);

uint32_t Texture_GetWidth(Texture *texture);

uint32_t Texture_GetHeight(Texture *texture);

uint32_t Texture_GetTextureId(Texture *texture);

uint32_t Texture_GetFramebufferId(Texture *texture);
