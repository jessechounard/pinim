#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <BatchRenderer.h>
#define GAME_MATH_IMPLEMENTATION
#include <GameMath.h>
#include <GraphicsDevice.h>
#include <ShaderProgram.h>
#include <Texture.h>
#include <VertexBuffer.h>

typedef struct Vertex {
    float x, y, u, v, r, g, b, a;
} Vertex;

static const uint32_t WINDOW_WIDTH = 1280;
static const uint32_t WINDOW_HEIGHT = 720;

typedef struct {
    SDL_Window *window;
    GraphicsDevice *graphicsDevice;
    BatchRenderer *batchRenderer;
    Texture *renderTarget;
    Texture *texture;
    float time;
    uint64_t currentTime;
} Context;

SDL_AppResult SDL_AppIterate(void *state) {
    if (state == NULL) {
        return SDL_APP_FAILURE;
    }

    Context *context = (Context *)state;
    uint64_t newTime = SDL_GetPerformanceCounter();
    float deltaSeconds = (newTime - context->currentTime) / (float)SDL_GetPerformanceFrequency();
    context->currentTime = newTime;
    context->time += deltaSeconds;

    GraphicsDevice_BeginFrame(context->graphicsDevice);

    GraphicsDevice_BindRenderTarget(context->graphicsDevice, context->renderTarget, true);

    GraphicsDevice_ClearScreen(context->graphicsDevice, &(Color){.r = 0, .g = 0, .b = 1, .a = 1});

    BatchRenderer_Begin(context->batchRenderer,
        BLEND_MODE_PREMULTIPLIED_ALPHA,
        context->texture,
        NULL,
        MATRIX4_IDENTITY);

    BatchRenderer_BatchQuadUV(context->batchRenderer,
        (Vector2){0, 0},
        (Vector2){1, 1},
        (Vector2){0, 0},
        (Vector2){WINDOW_WIDTH, WINDOW_HEIGHT},
        NULL);

    BatchRenderer_End(context->batchRenderer);

    GraphicsDevice_UnbindRenderTarget(context->graphicsDevice, true);

    GraphicsDevice_ClearScreen(context->graphicsDevice, &(Color){.r = 0, .g = 0, .b = 0, .a = 1});

    BatchRenderer_Begin(
        context->batchRenderer, BLEND_MODE_NONE, context->renderTarget, NULL, MATRIX4_IDENTITY);

    BatchRenderer_BatchQuad(context->batchRenderer,
        NULL,
        (float[]){640, 360},
        0,
        (float[]){1, 1},
        (float[]){0.5f, 0.5f},
        UVMODE_NORMAL,
        NULL);

    BatchRenderer_End(context->batchRenderer);

    BatchRenderer_Begin(context->batchRenderer,
        BLEND_MODE_PREMULTIPLIED_ALPHA,
        context->texture,
        NULL,
        MATRIX4_IDENTITY);

    BatchRenderer_BatchQuad(context->batchRenderer,
        NULL,
        (float[]){640, 360},
        context->time,
        (float[]){1, 1},
        (float[]){0.5f, 0.5f},
        UVMODE_FLIP_HORIZONTAL,
        NULL);

    BatchRenderer_End(context->batchRenderer);

    GraphicsDevice_EndFrame(context->graphicsDevice);

    SDL_GL_SwapWindow(context->window);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void **state, int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed");
        return SDL_APP_FAILURE;
    }

    Context *context = SDL_calloc(1, sizeof(Context));
    if (!context) {
        SDL_Log("SDL_calloc failed");
        return SDL_APP_FAILURE;
    }

    *state = context;
    context->time = 0;
    context->currentTime = SDL_GetPerformanceCounter();

    uint32_t windowFlags = GraphicsDevice_PrepareSDLWindowAttributes(GRAPHICS_API_OPENGL);

    context->window = SDL_CreateWindow("test", WINDOW_WIDTH, WINDOW_HEIGHT, windowFlags);
    if (context->window == NULL) {
        SDL_Log("SDL_CreateWindow failed");
        return SDL_APP_FAILURE;
    }

    context->graphicsDevice =
        GraphicsDevice_Create(GRAPHICS_API_OPENGL, context->window, VERTICAL_SYNC_DISABLED);
    if (context->graphicsDevice == NULL) {
        SDL_Log("GraphicsDevice_Create failed");
        return SDL_APP_FAILURE;
    }

    context->texture = Texture_Create(
        context->graphicsDevice, "Content/texture.png", TEXTURE_FILTER_LINEAR, TEXTURE_TYPE_NORMAL);
    if (context->texture == NULL) {
        SDL_Log("Texture_Create failed");
        return SDL_APP_FAILURE;
    }

    context->renderTarget = Texture_CreateFromPixelData(context->graphicsDevice,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        NULL,
        0,
        TEXTURE_FILTER_LINEAR,
        TEXTURE_TYPE_RENDERTARGET);
    if (context->renderTarget == NULL) {
        SDL_Log("Texture_CreateFromPixelData failed");
        return SDL_APP_FAILURE;
    }

    context->batchRenderer = BatchRenderer_Create(context->graphicsDevice, 1000);
    if (context->batchRenderer == NULL) {
        SDL_Log("BatchRenderer_Create failed");
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *state, SDL_Event *event) {
    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *state, SDL_AppResult result) {
    if (state != NULL) {
        Context *context = (Context *)state;
        if (context->batchRenderer != NULL) {
            BatchRenderer_Destroy(context->batchRenderer);
        }
        if (context->renderTarget != NULL) {
            Texture_Destroy(context->renderTarget);
        }
        if (context->texture != NULL) {
            Texture_Destroy(context->texture);
        }
        if (context->graphicsDevice != NULL) {
            GraphicsDevice_Destroy(context->graphicsDevice);
        }
        if (context->window != NULL) {
            SDL_DestroyWindow(context->window);
        }
        if (context != NULL) {
            SDL_free(context);
        }
    }
}