#pragma once

#include "../common.h"

#define MAX_INSTANCES 1024

struct InstanceData
{
	mat4 Transform;
};

uniformbuffer Instances
{
	mat4 gInstances[MAX_INSTANCES];
};

#define INST_TRANSFORM gInstances[gl_InstanceID]
