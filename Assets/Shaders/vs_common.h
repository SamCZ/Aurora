#pragma once

#include "common.h"

uniformbuffer BaseVSData
{
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
	mat4 ProjectionViewMatrix;
	mat4 ModelTransform;
};