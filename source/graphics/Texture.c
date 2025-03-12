#include <glad/gl.h>
#include <SDL3/SDL.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <GraphicsDevice.h>
#include <Texture.h>

struct Texture {
    TextureFilter textureFilter;
    TextureType textureType;
    uint32_t width;
    uint32_t height;
    uint32_t textureId;
    uint32_t fbo;
};

static bool Texture_Initialize(Texture *texture, TextureType textureType, uint32_t width,
    uint32_t height, uint8_t *pixelData, uint32_t dataLength, TextureFilter textureFilter) {
    texture->width = width;
    texture->height = height;
    texture->textureType = textureType;

    glGenTextures(1, &texture->textureId);

    Texture_SetTextureFilter(texture, textureFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);

    if (textureType == TEXTURE_TYPE_RENDERTARGET) {
        int currentFramebufferObject;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFramebufferObject);

        glGenFramebuffers(1, &texture->fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, texture->fbo);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->textureId, 0);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            SDL_Log("Failed to create render target texture");
            glDeleteTextures(1, &texture->textureId);
            return NULL;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, currentFramebufferObject);
    }

    return texture;
}

Texture *Texture_Create(GraphicsDevice *graphicsDevice, char *fileName, TextureFilter textureFilter,
    TextureType textureType) {
    assert(graphicsDevice != NULL);
    assert(fileName != NULL);

    int imageWidth, imageHeight, imageChannels;
    uint8_t *imagePixels = stbi_load(fileName, &imageWidth, &imageHeight, &imageChannels, 4);
    if (imagePixels == NULL) {
        SDL_Log("stbi_load failed: %s", fileName);
        return NULL;
    }

    Texture *texture = SDL_calloc(1, sizeof(Texture));
    if (texture == NULL) {
        SDL_Log("SDL_calloc failed");
        stbi_image_free(imagePixels);
        return NULL;
    }

    if (!Texture_Initialize(texture,
            TEXTURE_TYPE_NORMAL,
            imageWidth,
            imageHeight,
            imagePixels,
            imageWidth * imageHeight * 4,
            textureFilter)) {
        SDL_Log("Texture_Initialize failed");
        SDL_free(texture);
        stbi_image_free(imagePixels);
        return NULL;
    }

    stbi_image_free(imagePixels);

    return texture;
}

Texture *Texture_CreateFromBuffer(GraphicsDevice *graphicsDevice, void *buffer, uint32_t length,
    TextureFilter textureFilter, TextureType textureType) {
    assert(graphicsDevice != NULL);
    assert(buffer != NULL);
    assert(length > 0);

    int imageWidth, imageHeight, imageChannels;
    uint8_t *imagePixels =
        stbi_load_from_memory(buffer, length, &imageWidth, &imageHeight, &imageChannels, 4);
    if (imagePixels == NULL) {
        SDL_Log("stbi_load_from_memory failed");
        return NULL;
    }

    Texture *texture = SDL_calloc(1, sizeof(Texture));
    if (texture == NULL) {
        SDL_Log("SDL_calloc failed");
        stbi_image_free(imagePixels);
        return NULL;
    }

    if (!Texture_Initialize(texture,
            TEXTURE_TYPE_NORMAL,
            imageWidth,
            imageHeight,
            imagePixels,
            imageWidth * imageHeight * 4,
            textureFilter)) {
        SDL_Log("Texture_Initialize failed");
        SDL_free(texture);
        stbi_image_free(imagePixels);
        return NULL;
    }

    stbi_image_free(imagePixels);

    return texture;
}

Texture *Texture_CreateFromPixelData(GraphicsDevice *graphicsDevice, uint32_t width,
    uint32_t height, uint8_t *pixelData, uint32_t dataLength, TextureFilter textureFilter,
    TextureType textureType) {
    assert(graphicsDevice != NULL);
    assert(width > 0);
    assert(height > 0);
    if (pixelData != NULL) {
        assert(dataLength >= width * height * 4);
    }

    Texture *texture = SDL_calloc(1, sizeof(Texture));
    if (texture == NULL) {
        SDL_Log("SDL_calloc failed");
        return NULL;
    }

    if (!Texture_Initialize(
            texture, textureType, width, height, pixelData, dataLength, textureFilter)) {
        SDL_Log("Texture_Initialize failed");
        SDL_free(texture);
    }
    return texture;
}

void Texture_Destroy(Texture *texture) {
    assert(texture != NULL);

    if (texture->textureType == TEXTURE_TYPE_RENDERTARGET) {
        glDeleteFramebuffers(1, &texture->fbo);
    }

    glDeleteTextures(1, &texture->textureId);

    SDL_free(texture);
}

void Texture_SetTextureData(Texture *texture, uint32_t x, uint32_t y, uint32_t w, uint32_t h,
    uint8_t *pixelData, uint32_t dataLength) {
    assert(texture != NULL);
    assert(pixelData != NULL);
    assert(x + w <= texture->width);
    assert(y + h <= texture->height);
    assert(dataLength == w * h * 4);

    glBindTexture(GL_TEXTURE_2D, texture->textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
}

TextureFilter Texture_GetTextureFilter(Texture *texture) {
    assert(texture != NULL);
    return texture->textureFilter;
}

void Texture_SetTextureFilter(Texture *texture, TextureFilter textureFilter) {
    texture->textureFilter = textureFilter;
    assert(texture != NULL);

    glBindTexture(GL_TEXTURE_2D, texture->textureId);

    switch (textureFilter) {
    case TEXTURE_FILTER_LINEAR:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;

    case TEXTURE_FILTER_POINT:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;

    default:
        SDL_Log("Texture_SetTextureFilter error: Unsupported TextureFilter type");
        return;
    }
}

TextureType Texture_GetTextureType(Texture *texture) {
    assert(texture != NULL);
    return texture->textureType;
}

uint32_t Texture_GetWidth(Texture *texture) {
    assert(texture != NULL);
    return texture->width;
}

uint32_t Texture_GetHeight(Texture *texture) {
    assert(texture != NULL);
    return texture->height;
}

uint32_t Texture_GetTextureId(Texture *texture) {
    assert(texture != NULL);
    return texture->textureId;
}

uint32_t Texture_GetFramebufferId(Texture *texture) {
    assert(texture != NULL);
    return texture->fbo;
}
