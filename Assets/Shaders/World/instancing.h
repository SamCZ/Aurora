#pragma once

#include "../common.h"

#define MAX_INSTANCES 1024

struct ObjectInstanceData
{
	mat4 ModelMatrix;
};

layout(std140) uniformbuffer Instances
{
	ObjectInstanceData gInstances[MAX_INSTANCES];
};