#pragma once

#include "../common.h"

#define MAX_INSTANCES 1024

layout(std140) uniformbuffer Instances
{
	mat4 gInstances[MAX_INSTANCES];
};