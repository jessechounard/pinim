#pragma once

#include "GraphicsDevice.h"

typedef struct VertexShader VertexShader;
typedef struct FragmentShader FragmentShader;
typedef struct ShaderProgram ShaderProgram;

VertexShader *VertexShader_Create(GraphicsDevice *graphicsDevice, char *fileName);

VertexShader *VertexShader_CreateFromBuffer(
    GraphicsDevice *graphicsDevice, void *buffer, uint32_t length);

void VertexShader_Destroy(VertexShader *vertexShader);

FragmentShader *FragmentShader_Create(GraphicsDevice *graphicsDevice, char *fileName);

FragmentShader *FragmentShader_CreateFromBuffer(
    GraphicsDevice *graphicsDevice, void *buffer, uint32_t length);

void FragmentShader_Destroy(FragmentShader *fragmentShader);

ShaderProgram *ShaderProgram_Create(
    GraphicsDevice *graphicsDevice, VertexShader *vertexShader, FragmentShader *fragmentShader);
void ShaderProgram_Destroy(ShaderProgram *shaderProgram);

// bool ShaderProgram_SetParameterTexture2D(ShaderProgram *shaderProgram, Texture *texture, int
// slotNumber);

bool ShaderProgram_SetParameterMatrix4(
    ShaderProgram *shaderProgram, char *parameterName, float parameterValue[16]);

bool ShaderProgram_SetParameterInt(
    ShaderProgram *shaderProgram, char *parameterName, int parameterValue);

bool ShaderProgram_SetParameterInt2(
    ShaderProgram *shaderProgram, char *parameterName, int parameterValue[2]);

bool ShaderProgram_SetParameterInt3(
    ShaderProgram *shaderProgram, char *parameterName, int parameterValue[3]);

bool ShaderProgram_SetParameterInt4(
    ShaderProgram *shaderProgram, char *parameterName, int parameterValue[4]);

bool ShaderProgram_SetParameterFloat(
    ShaderProgram *shaderProgram, char *parameterName, float parameterValue);

bool ShaderProgram_SetParameterFloat2(
    ShaderProgram *shaderProgram, char *parameterName, float parameterValue[2]);

bool ShaderProgram_SetParameterFloat3(
    ShaderProgram *shaderProgram, char *parameterName, float parameterValue[3]);

bool ShaderProgram_SetParameterFloat4(
    ShaderProgram *shaderProgram, char *parameterName, float parameterValue[4]);

void ShaderProgram_ClearParameter(ShaderProgram *shaderProgram, char *parameterName);

void ShaderProgram_ApplyParameters(ShaderProgram *shaderProgram);

int32_t ShaderProgram_GetParameterLocation(ShaderProgram *shaderProgram, char *parameterName);
uint32_t ShaderProgram_GetParameterType(ShaderProgram *shaderProgram, char *parameterName);

int32_t ShaderProgram_GetAttributeLocation(ShaderProgram *shaderProgram, char *attributeName);
uint32_t ShaderProgram_GetAttributeType(ShaderProgram *shaderProgram, char *attributeName);

uint32_t ShaderProgram_GetShaderId(ShaderProgram *shaderProgram);