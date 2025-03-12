#pragma once

typedef enum BlendMode {
    BLEND_MODE_INVALID = -1,
    BLEND_MODE_NONE,
    BLEND_MODE_ADDITIVE,
    BLEND_MODE_ALPHA,
    BLEND_MODE_PREMULTIPLIED_ALPHA,
} BlendMode;

typedef enum GraphicsAPI {
    GRAPHICS_API_OPENGL,
} GraphicsAPI;

typedef enum RenderPrimitiveType {
    RENDER_PRIMITIVE_TRIANGLES,
    RENDER_PRIMITIVE_TRIANGLE_STRIP,
    RENDER_PRIMITIVE_LINES,
    RENDER_PRIMITIVE_LINE_STRIP,
    RENDER_PRIMITIVE_POINTS,
} RenderPrimitiveType;

typedef enum TextureFilter {
    TEXTURE_FILTER_LINEAR,
    TEXTURE_FILTER_POINT,
} TextureFilter;

typedef enum TextureType {
    TEXTURE_TYPE_NORMAL,
    TEXTURE_TYPE_RENDERTARGET,
} TextureType;

typedef enum UVMode {
    UVMODE_NORMAL = 0,
    UVMODE_ROTATED_CW90 = 1 << 1,    // Matches texture packer rotation
    UVMODE_FLIP_HORIZONTAL = 1 << 2, // flipped left to right
    UVMODE_FLIP_VERTICAL = 1 << 3,   // flipped top to bottom
} UVMode;

typedef enum VertexBufferType {
    VERTEX_BUFFER_STATIC,
    VERTEX_BUFFER_DYNAMIC,
} VertexBufferType;

typedef enum VerticalSyncType {
    VERTICAL_SYNC_ADAPTIVE,
    VERTICAL_SYNC_ENABLED,
    VERTICAL_SYNC_DISABLED,
} VerticalSyncType;

typedef struct Rectangle {
    int x, y, width, height;
} Rectangle;

typedef struct Color {
    float r, g, b, a;
} Color;

typedef struct Vertex2d {
    float x, y;
    float u, v;
    float r, g, b, a;
} Vertex2d;

typedef struct BatchRenderer BatchRenderer;
typedef struct Color Color;
typedef struct FragmentShader FragmentShader;
typedef struct GraphicsDevice GraphicsDevice;
typedef struct ShaderProgram ShaderProgram;
typedef struct Texture Texture;
typedef struct Vertex2d Vertex2d;
typedef struct VertexBuffer VertexBuffer;
typedef struct VertexShader VertexShader;
