#include <assert.h>

#include <glad/gl.h>
#include <SDL3/SDL.h>

#include "GraphicsDevice.h"
#include "ShaderProgram.h"
#include "VertexBuffer.h"

struct GraphicsDevice {
    Rectangle viewport;
    Color clearColor;

    SDL_GLContext openglContext;
    BlendMode blendMode;

    int windowWidth;
    int windowHeight;

    bool scissorsEnabled;
    Rectangle scissorsRectangle;

    uint32_t defaultFramebufferObject;
    uint32_t currentFramebufferObject;
};

uint32_t GraphicsDevice_PrepareSDLWindowAttributes(GraphicsAPI api) {
    switch (api) {
    case GRAPHICS_API_OPENGL:
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
        return SDL_WINDOW_OPENGL;

    default:
        SDL_Log("Unsupported GraphicsAPI type.");
        return 0;
    }
}

GraphicsDevice *GraphicsDevice_Create(
    GraphicsAPI api, SDL_Window *window, VerticalSyncType vsyncType) {
    assert(api == GRAPHICS_API_OPENGL);
    assert(window != NULL);

    GraphicsDevice *graphicsDevice = SDL_malloc(sizeof(GraphicsDevice));

    graphicsDevice->openglContext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, graphicsDevice->openglContext);

    switch (vsyncType) {
    case VERTICAL_SYNC_ADAPTIVE:
        if (!SDL_GL_SetSwapInterval(-1)) {
            SDL_GL_SetSwapInterval(1);
        }
        break;

    case VERTICAL_SYNC_ENABLED:
        SDL_GL_SetSwapInterval(1);
        break;

    case VERTICAL_SYNC_DISABLED:
        SDL_GL_SetSwapInterval(0);
        break;
    }

    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        SDL_Log("gladLoadGL failed");
        SDL_free(graphicsDevice);
        return NULL;
    }

    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    SDL_GetWindowSizeInPixels(window, &graphicsDevice->windowWidth, &graphicsDevice->windowHeight);
    glViewport(0, 0, graphicsDevice->windowWidth, graphicsDevice->windowHeight);
    graphicsDevice->viewport.x = 0;
    graphicsDevice->viewport.y = 0;
    graphicsDevice->viewport.width = graphicsDevice->windowWidth;
    graphicsDevice->viewport.height = graphicsDevice->windowHeight;

    glClearColor(0, 0, 0, 1);
    graphicsDevice->clearColor = (Color){.r = 0, .g = 0, .b = 0, .a = 0};

    graphicsDevice->blendMode = BLEND_MODE_INVALID;
    GraphicsDevice_SetBlendMode(graphicsDevice, BLEND_MODE_PREMULTIPLIED_ALPHA);

    graphicsDevice->scissorsEnabled = false;

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (int32_t *)&graphicsDevice->defaultFramebufferObject);
    graphicsDevice->currentFramebufferObject = graphicsDevice->defaultFramebufferObject;

    SDL_Log("GL: OpenGL device information:");
    SDL_Log("  Vendor:   %s", (const char *)glGetString(GL_VENDOR));
    SDL_Log("  Renderer: %s", (const char *)glGetString(GL_RENDERER));
    SDL_Log("  Version:  %s", (const char *)glGetString(GL_VERSION));
    SDL_Log("  GLSL:     %s", (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION));

    return graphicsDevice;
}

void GraphicsDevice_Destroy(GraphicsDevice *device) {
    assert(device != NULL);

    SDL_free(device);
}

void GraphicsDevice_SetViewport(GraphicsDevice *graphicsDevice, Rectangle *viewport) {
    graphicsDevice->viewport = *viewport;
    glViewport(viewport->x, viewport->y, viewport->width, viewport->height);
}

void GraphicsDevice_GetViewport(GraphicsDevice *graphicsDevice, Rectangle *viewport) {
    *viewport = graphicsDevice->viewport;
}

void GraphicsDevice_ClearScreen(GraphicsDevice *graphicsDevice, Color *color) {
    if (graphicsDevice->scissorsEnabled) {
        glDisable(GL_SCISSOR_TEST);
    }

    if (color->r != graphicsDevice->clearColor.r || color->g != graphicsDevice->clearColor.g ||
        color->b != graphicsDevice->clearColor.b || color->a != graphicsDevice->clearColor.a) {
        glClearColor(color->r, color->g, color->b, color->a);
        graphicsDevice->clearColor = *color;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    if (graphicsDevice->scissorsEnabled) {
        glEnable(GL_SCISSOR_TEST);
    }
}

void GraphicsDevice_SetBlendMode(GraphicsDevice *graphicsDevice, BlendMode blendMode) {
    if (graphicsDevice->blendMode == blendMode) {
        return;
    }

    switch (blendMode) {
    case BLEND_MODE_NONE:
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
        break;
    case BLEND_MODE_ADDITIVE:
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
        break;
    case BLEND_MODE_ALPHA:
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
        break;
    case BLEND_MODE_PREMULTIPLIED_ALPHA:
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
        break;
    default:
        SDL_Log("Unsupported BlendMode type");
        return;
    }

    graphicsDevice->blendMode = blendMode;
}

void GraphicsDevice_EnableScissorsRectangle(
    GraphicsDevice *graphicsDevice, Rectangle *scissorsRectangle) {
    assert(graphicsDevice != NULL);

    graphicsDevice->scissorsEnabled = true;
    graphicsDevice->scissorsRectangle = *scissorsRectangle;

    if (!GraphicsDevice_IsUsingRenderTarget(graphicsDevice)) {
        graphicsDevice->scissorsRectangle.y = graphicsDevice->viewport.height -
                                              graphicsDevice->scissorsRectangle.y -
                                              graphicsDevice->scissorsRectangle.height;
    }

    glEnable(GL_SCISSOR_TEST);
    glScissor(scissorsRectangle->x,
        scissorsRectangle->y,
        scissorsRectangle->width,
        scissorsRectangle->height);
}

void GraphicsDevice_DisableScissorsRectangle(GraphicsDevice *graphicsDevice) {
    assert(graphicsDevice != NULL);

    glDisable(GL_SCISSOR_TEST);
    graphicsDevice->scissorsEnabled = false;
}

bool GraphicsDevice_IsUsingRenderTarget(GraphicsDevice *graphicsDevice) {
    assert(graphicsDevice != NULL);

    return graphicsDevice->defaultFramebufferObject != graphicsDevice->currentFramebufferObject;
}

void GraphicsDevice_ReadPixels(GraphicsDevice *graphicsDevice, uint32_t x, uint32_t y,
    uint32_t width, uint32_t height, uint8_t *pixels) {
    assert(graphicsDevice != NULL);
    assert(width > 0);
    assert(height > 0);
    assert(pixels != NULL);

    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

void GraphicsDevice_ApplyShaderProgram(
    GraphicsDevice *graphicsDevice, ShaderProgram *shaderProgram) {
    assert(graphicsDevice != NULL);
    assert(shaderProgram != NULL);

    glUseProgram(ShaderProgram_GetShaderId(shaderProgram));
}

void GraphicsDevice_BeginFrame(GraphicsDevice *graphicsDevice) {
    assert(graphicsDevice != NULL);
}
void GraphicsDevice_EndFrame(GraphicsDevice *graphicsDevice) {
    assert(graphicsDevice != NULL);
}

void GraphicsDevice_DrawPrimitives(GraphicsDevice *graphicsDevice, VertexBuffer *vertexBuffer,
    RenderPrimitiveType primitiveType, uint32_t vertexStart, uint32_t primitiveCount) {
    assert(graphicsDevice != NULL);
    assert(vertexBuffer != NULL);
    assert(primitiveCount > 0);

    glBindVertexArray(VertexBuffer_GetArrayId(vertexBuffer));
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer_GetBufferId(vertexBuffer));

    int vertexCount;
    GLenum mode;

    switch (primitiveType) {
    case RENDER_PRIMITIVE_TRAINGLES:
        vertexCount = primitiveCount * 3;
        mode = GL_TRIANGLES;
        break;
    case RENDER_PRIMITIVE_TRIANGLE_STRIP:
        vertexCount = primitiveCount + 2;
        mode = GL_TRIANGLE_STRIP;
        break;
    case RENDER_PRIMITIVE_LINES:
        vertexCount = primitiveCount * 2;
        mode = GL_LINES;
        break;
    case RENDER_PRIMITIVE_LINE_STRIP:
        vertexCount = primitiveCount + 1;
        mode = GL_LINE_STRIP;
        break;
    case RENDER_PRIMITIVE_POINTS:
        vertexCount = primitiveCount;
        mode = GL_POINTS;
        break;
    default:
        SDL_Log("Unsupported PrimitiveType: %d", primitiveType);
        return;
    }

    glDrawArrays(mode, vertexStart, vertexCount);
}
