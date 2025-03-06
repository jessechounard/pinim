#include <assert.h>
#include <glad/gl.h>
#include <SDL3/SDL.h>

#include "ShaderProgram.h"

typedef enum ShaderParameterType {
    SHADER_PARAMETER_INVALID = 0,
    SHADER_PARAMETER_TEXTURE2D = GL_SAMPLER_2D,
    SHADER_PARAMETER_FLOAT_MAT4 = GL_FLOAT_MAT4,
    SHADER_PARAMETER_FLOAT = GL_FLOAT,
    SHADER_PARAMETER_FLOAT_VEC2 = GL_FLOAT_VEC2,
    SHADER_PARAMETER_FLOAT_VEC3 = GL_FLOAT_VEC3,
    SHADER_PARAMETER_FLOAT_VEC4 = GL_FLOAT_VEC4,
    SHADER_PARAMETER_INT = GL_INT,
    SHADER_PARAMETER_INT_VEC2 = GL_INT_VEC2,
    SHADER_PARAMETER_INT_VEC3 = GL_INT_VEC3,
    SHADER_PARAMETER_INT_VEC4 = GL_INT_VEC4,
} ShaderParameterType;

typedef struct ShaderParameterValue {
    union {
        struct {
            float matrix[16];
        };
        struct {
            uint32_t textureId;
            int32_t slot;
        };
        struct {
            float f[4];
        };
        struct {
            int i[4];
        };
    };

    ShaderParameterType type;
} ShaderParameterValue;

struct VertexShader {
    uint32_t id;
};

struct FragmentShader {
    uint32_t id;
};

typedef struct ShaderDetail {
    char name[256];
    int32_t location;
    ShaderParameterType type;
} ShaderDetail;

struct ShaderProgram {
    uint32_t id;
    ShaderDetail *attributes;
    ShaderDetail *parameters;
    ShaderParameterValue *parameterValues;
    int attributeCount;
    int parameterCount;
};

VertexShader *VertexShader_Create(GraphicsDevice *graphicsDevice, char *fileName) {
    assert(graphicsDevice != NULL);
    assert(fileName != NULL);

    size_t dataSize;
    void *data = SDL_LoadFile(fileName, &dataSize);

    if (data == NULL) {
        SDL_Log("SDL_LoadFile failed %s", fileName);
        return NULL;
    }
    VertexShader *vertexShader = VertexShader_CreateFromBuffer(graphicsDevice, data, dataSize);
    SDL_free(data);
    return vertexShader;
}

VertexShader *VertexShader_CreateFromBuffer(
    GraphicsDevice *graphicsDevice, void *buffer, uint32_t length) {
    assert(graphicsDevice != NULL);
    assert(buffer != NULL);
    assert(length > 0);

    VertexShader *vertexShader = SDL_malloc(sizeof(VertexShader));
    if (vertexShader == NULL) {
        SDL_Log("SDL_malloc failed");
        return NULL;
    }

    vertexShader->id = glCreateShader(GL_VERTEX_SHADER);
    if (vertexShader->id == 0) {
        SDL_Log("glCreateShader(GL_VERTEX_SHADER) failed");
        SDL_free(vertexShader);
        return NULL;
    }

    glShaderSource(vertexShader->id, 1, (const GLchar *const *)&buffer, (GLint *)&length);
    glCompileShader(vertexShader->id);

    int status;
    glGetShaderiv(vertexShader->id, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char infoLog[1024];
        int logLength;
        glGetShaderInfoLog(vertexShader->id, 1024, &logLength, infoLog);
        SDL_Log("VertexShader compilation failed. Compiler output:\n%s", infoLog);
        glDeleteShader(vertexShader->id);
        SDL_free(vertexShader);
        return NULL;
    }

    return vertexShader;
}

void VertexShader_Destroy(VertexShader *vertexShader) {
    assert(vertexShader != NULL);

    glDeleteShader(vertexShader->id);
    SDL_free(vertexShader);
}

FragmentShader *FragmentShader_Create(GraphicsDevice *graphicsDevice, char *fileName) {
    assert(graphicsDevice != NULL);
    assert(fileName != NULL);

    size_t dataSize;
    void *data = SDL_LoadFile(fileName, &dataSize);

    if (data == NULL) {
        SDL_Log("SDL_LoadFile failed %s", fileName);
        return NULL;
    }
    FragmentShader *fragmentShader =
        FragmentShader_CreateFromBuffer(graphicsDevice, data, dataSize);
    SDL_free(data);
    return fragmentShader;
}

FragmentShader *FragmentShader_CreateFromBuffer(
    GraphicsDevice *graphicsDevice, void *buffer, uint32_t length) {
    assert(graphicsDevice != NULL);
    assert(buffer != NULL);
    assert(length > 0);

    FragmentShader *fragmentShader = SDL_malloc(sizeof(FragmentShader));
    if (fragmentShader == NULL) {
        SDL_Log("SDL_malloc failed");
        return NULL;
    }

    fragmentShader->id = glCreateShader(GL_FRAGMENT_SHADER);
    if (fragmentShader->id == 0) {
        SDL_Log("glCreateShader(GL_FRAGMENT_SHADER) failed");
        SDL_free(fragmentShader);
        return NULL;
    }

    glShaderSource(fragmentShader->id, 1, (const GLchar *const *)&buffer, (GLint *)&length);
    glCompileShader(fragmentShader->id);

    int status;
    glGetShaderiv(fragmentShader->id, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char infoLog[1024];
        int logLength;
        glGetShaderInfoLog(fragmentShader->id, 1024, &logLength, infoLog);
        SDL_Log("FragmentShader compilation failed. Compiler output:\n%s", infoLog);
        glDeleteShader(fragmentShader->id);
        SDL_free(fragmentShader);
        return NULL;
    }

    return fragmentShader;
}

void FragmentShader_Destroy(FragmentShader *fragmentShader) {
    assert(fragmentShader != NULL);

    glDeleteShader(fragmentShader->id);
    SDL_free(fragmentShader);
}

ShaderProgram *ShaderProgram_Create(
    GraphicsDevice *graphicsDevice, VertexShader *vertexShader, FragmentShader *fragmentShader) {
    assert(graphicsDevice != NULL);
    assert(vertexShader != NULL);
    assert(fragmentShader != NULL);

    ShaderProgram *shaderProgram = SDL_malloc(sizeof(ShaderProgram));
    if (shaderProgram == NULL) {
        SDL_Log("SDL_malloc failed");
        return NULL;
    }

    shaderProgram->attributes = NULL;
    shaderProgram->attributeCount = 0;
    shaderProgram->parameters = NULL;
    shaderProgram->parameterValues = NULL;
    shaderProgram->parameterCount = 0;

    shaderProgram->id = glCreateProgram();
    if (shaderProgram->id == 0) {
        SDL_Log("glCreateProgram failed");
        SDL_free(shaderProgram);
        return NULL;
    }

    glAttachShader(shaderProgram->id, vertexShader->id);
    glAttachShader(shaderProgram->id, fragmentShader->id);
    glLinkProgram(shaderProgram->id);

    int status;
    glGetProgramiv(shaderProgram->id, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char infoLog[1024];
        int logLength;
        glGetProgramInfoLog(shaderProgram->id, 1024, &logLength, infoLog);
        glDeleteProgram(shaderProgram->id);
        SDL_Log("ShaderProgram linking failed. Linker output:\n%s", infoLog);
        glDeleteShader(shaderProgram->id);
        SDL_free(shaderProgram);
        return NULL;
    }

    int uniformCount;
    glGetProgramiv(shaderProgram->id, GL_ACTIVE_UNIFORMS, &uniformCount);

    shaderProgram->parameters = SDL_calloc(uniformCount, sizeof(ShaderDetail));
    if (shaderProgram->parameters == NULL) {
        SDL_Log("SDL_calloc failed");
        return NULL;
    }

    shaderProgram->parameterValues = SDL_malloc(uniformCount * sizeof(ShaderParameterValue));
    if (shaderProgram->parameterValues == NULL) {
        SDL_Log("SDL_calloc failed");
        return NULL;
    }

    shaderProgram->parameterCount = uniformCount;

    for (int i = 0; i < uniformCount; i++) {
        int size;

        glGetActiveUniform(shaderProgram->id,
            i,
            256,
            NULL,
            &size,
            &shaderProgram->parameters[i].type,
            shaderProgram->parameters[i].name);
        shaderProgram->parameters[i].location =
            glGetUniformLocation(shaderProgram->id, shaderProgram->parameters[i].name);
        shaderProgram->parameterValues[i].type = SHADER_PARAMETER_INVALID;
    }

    int attributeCount;
    glGetProgramiv(shaderProgram->id, GL_ACTIVE_ATTRIBUTES, &attributeCount);

    shaderProgram->attributes = SDL_calloc(attributeCount, sizeof(ShaderDetail));
    if (shaderProgram->attributes == NULL) {
        SDL_Log("SDL_calloc failed");
        return NULL;
    }

    shaderProgram->attributeCount = attributeCount;

    for (int i = 0; i < attributeCount; i++) {
        int size;

        glGetActiveAttrib(shaderProgram->id,
            i,
            256,
            NULL,
            &size,
            &shaderProgram->attributes[i].type,
            shaderProgram->attributes[i].name);
        shaderProgram->attributes[i].location =
            glGetAttribLocation(shaderProgram->id, shaderProgram->attributes[i].name);
    }

    uint32_t positionType = ShaderProgram_GetAttributeType(shaderProgram, "position");
    if (positionType != 0 && positionType != GL_FLOAT_VEC4) {
        SDL_Log("Shader program has an invalid type for attribute: position");
    }

    uint32_t colorType = ShaderProgram_GetAttributeType(shaderProgram, "color");
    if (colorType != 0 && colorType != GL_FLOAT_VEC4) {
        SDL_Log("Shader program has an invalid type for attribute: color");
    }

    uint32_t texcoordType = ShaderProgram_GetAttributeType(shaderProgram, "texcoord");
    if (texcoordType != 0 && texcoordType != GL_FLOAT_VEC2) {
        SDL_Log("Shader program has an invalid type for attribute: texcoord");
    }

    uint32_t projectionMatrixType =
        ShaderProgram_GetParameterType(shaderProgram, "ProjectionMatrix");
    if (projectionMatrixType != 0 && projectionMatrixType != GL_FLOAT_MAT4) {
        SDL_Log("Shader program has an invalid type for uniform: ProjectionMatrix");
    }

    uint32_t textureSamplerType = ShaderProgram_GetParameterType(shaderProgram, "TextureSampler");
    if (textureSamplerType != 0 && textureSamplerType != GL_SAMPLER_2D) {
        SDL_Log("Shader program has an invalid type for uniform: TextureSampler");
    }

    return shaderProgram;
}

void ShaderProgram_Destroy(ShaderProgram *shaderProgram) {
    assert(shaderProgram != NULL);

    SDL_free(shaderProgram->attributes);
    SDL_free(shaderProgram->parameterValues);
    SDL_free(shaderProgram->parameters);
    glDeleteShader(shaderProgram->id);
    SDL_free(shaderProgram);
}

int ShaderProgram_FindAttributeIndex(ShaderProgram *shaderProgram, char *attributeName) {
    assert(shaderProgram != NULL);
    assert(attributeName != NULL);

    for (int i = 0; i < shaderProgram->attributeCount; i++) {
        if (SDL_strcmp(shaderProgram->attributes[i].name, attributeName) == 0) {
            return i;
        }
    }

    return -1;
}

int ShaderProgram_FindParameterIndex(ShaderProgram *shaderProgram, char *parameterName) {
    assert(shaderProgram != NULL);
    assert(parameterName != NULL);

    for (int i = 0; i < shaderProgram->parameterCount; i++) {
        if (SDL_strcmp(shaderProgram->parameters[i].name, parameterName) == 0) {
            return i;
        }
    }

    return -1;
}

bool ShaderProgram_SetParameterMatrix4(
    ShaderProgram *shaderProgram, char *parameterName, float parameterValue[16]) {
    assert(shaderProgram != NULL);
    assert(parameterName != NULL);

    int index = ShaderProgram_FindParameterIndex(shaderProgram, parameterName);
    if (index == -1) {
        return false;
    }

    ShaderDetail *parameter = &shaderProgram->parameters[index];
    ShaderParameterValue *value = &shaderProgram->parameterValues[index];

    if (parameter->type != SHADER_PARAMETER_FLOAT_MAT4) {
        return false;
    }

    value->type = parameter->type;
    for (int i = 0; i < 16; i++) {
        value->matrix[i] = parameterValue[i];
    }

    return true;
}

bool ShaderProgram_SetParameterFloat(
    ShaderProgram *shaderProgram, char *parameterName, float parameterValue) {
    assert(shaderProgram != NULL);
    assert(parameterName != NULL);

    int index = ShaderProgram_FindParameterIndex(shaderProgram, parameterName);
    if (index == -1) {
        return false;
    }

    ShaderDetail *parameter = &shaderProgram->parameters[index];
    ShaderParameterValue *value = &shaderProgram->parameterValues[index];

    if (parameter->type != SHADER_PARAMETER_FLOAT) {
        return false;
    }

    value->type = parameter->type;
    value->f[0] = parameterValue;

    return true;
}

bool ShaderProgram_SetParameterFloat2(
    ShaderProgram *shaderProgram, char *parameterName, float parameterValue[2]) {
    assert(shaderProgram != NULL);
    assert(parameterName != NULL);

    int index = ShaderProgram_FindParameterIndex(shaderProgram, parameterName);
    if (index == -1) {
        return false;
    }

    ShaderDetail *parameter = &shaderProgram->parameters[index];
    ShaderParameterValue *value = &shaderProgram->parameterValues[index];

    if (parameter->type != SHADER_PARAMETER_FLOAT_VEC2) {
        return false;
    }

    value->type = parameter->type;
    value->f[0] = parameterValue[0];
    value->f[1] = parameterValue[1];

    return true;
}

bool ShaderProgram_SetParameterFloat3(
    ShaderProgram *shaderProgram, char *parameterName, float parameterValue[3]) {
    assert(shaderProgram != NULL);
    assert(parameterName != NULL);

    int index = ShaderProgram_FindParameterIndex(shaderProgram, parameterName);
    if (index == -1) {
        return false;
    }

    ShaderDetail *parameter = &shaderProgram->parameters[index];
    ShaderParameterValue *value = &shaderProgram->parameterValues[index];

    if (parameter->type != SHADER_PARAMETER_FLOAT_VEC3) {
        return false;
    }

    value->type = parameter->type;
    value->f[0] = parameterValue[0];
    value->f[1] = parameterValue[1];
    value->f[2] = parameterValue[3];

    return true;
}

bool ShaderProgram_SetParameterFloat4(
    ShaderProgram *shaderProgram, char *parameterName, float parameterValue[4]) {
    assert(shaderProgram != NULL);
    assert(parameterName != NULL);

    int index = ShaderProgram_FindParameterIndex(shaderProgram, parameterName);
    if (index == -1) {
        return false;
    }

    ShaderDetail *parameter = &shaderProgram->parameters[index];
    ShaderParameterValue *value = &shaderProgram->parameterValues[index];

    if (parameter->type != SHADER_PARAMETER_FLOAT_VEC4) {
        return false;
    }

    value->type = parameter->type;
    value->f[0] = parameterValue[0];
    value->f[1] = parameterValue[1];
    value->f[2] = parameterValue[2];
    value->f[3] = parameterValue[3];

    return true;
}

bool ShaderProgram_SetParameterInt(
    ShaderProgram *shaderProgram, char *parameterName, int parameterValue) {
    assert(shaderProgram != NULL);
    assert(parameterName != NULL);

    int index = ShaderProgram_FindParameterIndex(shaderProgram, parameterName);
    if (index == -1) {
        return false;
    }

    ShaderDetail *parameter = &shaderProgram->parameters[index];
    ShaderParameterValue *value = &shaderProgram->parameterValues[index];

    if (parameter->type != SHADER_PARAMETER_INT) {
        return false;
    }

    value->type = parameter->type;
    value->i[0] = parameterValue;

    return true;
}

bool ShaderProgram_SetParameterInt2(
    ShaderProgram *shaderProgram, char *parameterName, int parameterValue[2]) {
    assert(shaderProgram != NULL);
    assert(parameterName != NULL);

    int index = ShaderProgram_FindParameterIndex(shaderProgram, parameterName);
    if (index == -1) {
        return false;
    }

    ShaderDetail *parameter = &shaderProgram->parameters[index];
    ShaderParameterValue *value = &shaderProgram->parameterValues[index];

    if (parameter->type != SHADER_PARAMETER_INT_VEC2) {
        return false;
    }

    value->type = parameter->type;
    value->i[0] = parameterValue[0];
    value->i[1] = parameterValue[1];

    return true;
}

bool ShaderProgram_SetParameterInt3(
    ShaderProgram *shaderProgram, char *parameterName, int parameterValue[3]) {
    assert(shaderProgram != NULL);
    assert(parameterName != NULL);

    int index = ShaderProgram_FindParameterIndex(shaderProgram, parameterName);
    if (index == -1) {
        return false;
    }

    ShaderDetail *parameter = &shaderProgram->parameters[index];
    ShaderParameterValue *value = &shaderProgram->parameterValues[index];

    if (parameter->type != SHADER_PARAMETER_INT_VEC3) {
        return false;
    }

    value->type = parameter->type;
    value->i[0] = parameterValue[0];
    value->i[1] = parameterValue[1];
    value->i[2] = parameterValue[2];

    return true;
}

bool ShaderProgram_SetParameterInt4(
    ShaderProgram *shaderProgram, char *parameterName, int parameterValue[4]) {
    assert(shaderProgram != NULL);
    assert(parameterName != NULL);

    int index = ShaderProgram_FindParameterIndex(shaderProgram, parameterName);
    if (index == -1) {
        return false;
    }

    ShaderDetail *parameter = &shaderProgram->parameters[index];
    ShaderParameterValue *value = &shaderProgram->parameterValues[index];

    if (parameter->type != SHADER_PARAMETER_INT_VEC4) {
        return false;
    }

    value->type = parameter->type;
    value->i[0] = parameterValue[0];
    value->i[1] = parameterValue[1];
    value->i[2] = parameterValue[2];
    value->i[3] = parameterValue[3];

    return true;
}

void ShaderProgram_ClearParameter(ShaderProgram *shaderProgram, char *parameterName) {
    assert(shaderProgram != NULL);
    assert(parameterName != NULL);

    int index = ShaderProgram_FindParameterIndex(shaderProgram, parameterName);
    if (index != -1) {
        shaderProgram->parameterValues[index].type = SHADER_PARAMETER_INVALID;
    }
}

void ShaderProgram_ApplyParameters(ShaderProgram *shaderProgram) {
    assert(shaderProgram != NULL);

    for (int i = 0; i < shaderProgram->parameterCount; i++) {
        ShaderParameterValue *parameterValue = &shaderProgram->parameterValues[i];
        ShaderDetail *parameter = &shaderProgram->parameters[i];
        switch (parameterValue->type) {
        case SHADER_PARAMETER_TEXTURE2D:
            glActiveTexture(GL_TEXTURE0 + parameterValue->slot);
            glBindTexture(GL_TEXTURE_2D, parameterValue->textureId);
            glUniform1i(parameter->location, parameterValue->slot);
            break;
        case SHADER_PARAMETER_FLOAT_MAT4:
            glUniformMatrix4fv(parameter->location, 1, GL_FALSE, &parameterValue->matrix[0]);
            break;
        case SHADER_PARAMETER_FLOAT:

            glUniform1f(parameter->location, parameterValue->f[0]);
            break;
        case SHADER_PARAMETER_FLOAT_VEC2:
            glUniform2f(parameter->location, parameterValue->f[0], parameterValue->f[1]);
            break;
        case SHADER_PARAMETER_FLOAT_VEC3:
            glUniform3f(parameter->location,
                parameterValue->f[0],
                parameterValue->f[1],
                parameterValue->f[2]);
            break;
        case SHADER_PARAMETER_FLOAT_VEC4:
            glUniform4f(parameter->location,
                parameterValue->f[0],
                parameterValue->f[1],
                parameterValue->f[2],
                parameterValue->f[3]);
            break;
        case SHADER_PARAMETER_INT:
            glUniform1i(parameter->location, parameterValue->i[0]);

            break;
        case SHADER_PARAMETER_INT_VEC2:
            glUniform2i(parameter->location, parameterValue->i[0], parameterValue->i[1]);
            break;
        case SHADER_PARAMETER_INT_VEC3:
            glUniform3i(parameter->location,
                parameterValue->i[0],
                parameterValue->i[1],
                parameterValue->i[2]);
            break;
        case SHADER_PARAMETER_INT_VEC4:
            glUniform4i(parameter->location,
                parameterValue->i[0],
                parameterValue->i[1],
                parameterValue->i[2],
                parameterValue->i[3]);
            break;
        default:
            continue;
        }
    }
}

int32_t ShaderProgram_GetParameterLocation(ShaderProgram *shaderProgram, char *parameterName) {
    assert(shaderProgram != NULL);
    assert(parameterName != NULL);

    int index = ShaderProgram_FindParameterIndex(shaderProgram, parameterName);
    if (index != -1) {
        return shaderProgram->parameters[index].location;
    }

    return -1;
}

uint32_t ShaderProgram_GetParameterType(ShaderProgram *shaderProgram, char *parameterName) {
    assert(shaderProgram != NULL);
    assert(parameterName != NULL);

    int index = ShaderProgram_FindParameterIndex(shaderProgram, parameterName);
    if (index != -1) {
        return shaderProgram->parameters[index].type;
    }

    return 0;
}

int32_t ShaderProgram_GetAttributeLocation(ShaderProgram *shaderProgram, char *attributeName) {
    assert(shaderProgram != NULL);
    assert(attributeName != NULL);

    int index = ShaderProgram_FindAttributeIndex(shaderProgram, attributeName);
    if (index != -1) {
        return shaderProgram->attributes[index].location;
    }

    return -1;
}

uint32_t ShaderProgram_GetAttributeType(ShaderProgram *shaderProgram, char *attributeName) {
    assert(shaderProgram != NULL);
    assert(attributeName != NULL);

    int index = ShaderProgram_FindAttributeIndex(shaderProgram, attributeName);
    if (index != -1) {
        return shaderProgram->attributes[index].type;
    }

    return 0;
}

uint32_t ShaderProgram_GetShaderId(ShaderProgram *shaderProgram) {
    assert(shaderProgram != NULL);

    return shaderProgram->id;
}