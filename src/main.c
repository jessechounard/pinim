#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "GraphicsDevice.h"
#include "ShaderProgram.h"
#include "VertexBuffer.h"

static const uint32_t WINDOW_WIDTH = 1280;
static const uint32_t WINDOW_HEIGHT = 720;

typedef struct {
    SDL_Window *window;
    GraphicsDevice *graphicsDevice;
    ShaderProgram *shaderProgram;
    VertexBuffer *vertexBuffer;
    float time;
    uint64_t currentTime;
} Context;

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
    //"in vec2 v_texcoord;\n"
    "out vec4 fragColor;\n"
    // custom input from program
    //"uniform sampler2D TextureSampler;\n"
    //
    "void main()\n"
    "{\n"
    //"	fragColor = texture2D(TextureSampler, v_texcoord) * v_color;\n"
    "	fragColor = v_color;\n"
    "}\n";

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
    GraphicsDevice_ClearScreen(context->graphicsDevice, &(Color){.r = 0, .g = 0, .b = 0, .a = 1});

    float matrix[16];
    CreateOrthographicOffCenterMatrix(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0.1, 100, matrix);
    Vertex vertices[6];
    vertices[0] = (Vertex){.x = 0, .y = 0, .r = 1, .g = 1, .b = 1, .a = 1, .u = 0, .v = 0};
    vertices[1] = (Vertex){
        .x = WINDOW_WIDTH, .y = 0, .r = 1, .g = 1, .b = 1, .a = 1, .u = WINDOW_WIDTH, .v = 0};
    vertices[2] = (Vertex){
        .x = 0, .y = WINDOW_HEIGHT, .r = 1, .g = 1, .b = 1, .a = 1, .u = 0, .v = WINDOW_HEIGHT};
    vertices[3] = (Vertex){
        .x = 0, .y = WINDOW_HEIGHT, .r = 1, .g = 1, .b = 1, .a = 1, .u = 0, .v = WINDOW_HEIGHT};
    vertices[4] = (Vertex){
        .x = WINDOW_WIDTH, .y = 0, .r = 1, .g = 1, .b = 1, .a = 1, .u = WINDOW_WIDTH, .v = 0};
    vertices[5] = (Vertex){.x = WINDOW_WIDTH,
        .y = WINDOW_HEIGHT,
        .r = 1,
        .g = 1,
        .b = 1,
        .a = 1,
        .u = WINDOW_WIDTH,
        .v = WINDOW_HEIGHT};

    float resolution[2] = {WINDOW_WIDTH, WINDOW_HEIGHT};

    GraphicsDevice_SetBlendMode(context->graphicsDevice, BLEND_MODE_PREMULTIPLIED_ALPHA);
    GraphicsDevice_ApplyShaderProgram(context->graphicsDevice, context->shaderProgram);
    ShaderProgram_SetParameterMatrix4(context->shaderProgram, "ProjectionMatrix", matrix);
    ShaderProgram_SetParameterFloat2(context->shaderProgram, "iResolution", resolution);
    ShaderProgram_SetParameterFloat(context->shaderProgram, "iTime", context->time);

    ShaderProgram_ApplyParameters(context->shaderProgram);
    VertexBuffer_SetVertexData(context->vertexBuffer, context->shaderProgram, vertices, 6);
    GraphicsDevice_DrawPrimitives(
        context->graphicsDevice, context->vertexBuffer, RENDER_PRIMITIVE_TRAINGLES, 0, 2);

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

    SDL_Log("About to create graphics device");

    context->graphicsDevice =
        GraphicsDevice_Create(GRAPHICS_API_OPENGL, context->window, VERTICAL_SYNC_ADAPTIVE);
    if (context->graphicsDevice == NULL) {
        SDL_Log("GraphicsDevice_Create failed");
        return SDL_APP_FAILURE;
    }

    VertexShader *vertexShader = VertexShader_CreateFromBuffer(
        context->graphicsDevice, defaultVertexShaderSource, SDL_strlen(defaultVertexShaderSource));
    if (vertexShader == NULL) {
        SDL_Log("VertexShader_CreateFromBuffer failed");
        return SDL_APP_FAILURE;
    }

    FragmentShader *fragmentShader =
        FragmentShader_Create(context->graphicsDevice, "Content/Shaders/ShaderArt.frag");
    if (fragmentShader == NULL) {
        SDL_Log("FragmentShader_CreateFromBuffer failed");
        VertexShader_Destroy(vertexShader);
        return SDL_APP_FAILURE;
    }

    context->shaderProgram =
        ShaderProgram_Create(context->graphicsDevice, vertexShader, fragmentShader);
    if (context->shaderProgram == NULL) {
        SDL_Log("ShaderProgram_Create failed");
        FragmentShader_Destroy(fragmentShader);
        VertexShader_Destroy(vertexShader);
        return SDL_APP_FAILURE;
    }

    FragmentShader_Destroy(fragmentShader);
    VertexShader_Destroy(vertexShader);

    context->vertexBuffer = VertexBuffer_Create(VERTEX_BUFFER_DYNAMIC, 4096);
    if (context->vertexBuffer == NULL) {
        SDL_Log("VertexBuffer_Create failed");
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
        VertexBuffer_Destroy(context->vertexBuffer);
        ShaderProgram_Destroy(context->shaderProgram);
        GraphicsDevice_Destroy(context->graphicsDevice);
        SDL_DestroyWindow(context->window);
        SDL_free(context);
    }
}