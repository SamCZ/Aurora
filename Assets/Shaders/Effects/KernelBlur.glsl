layout(local_size_x = 16, local_size_y = 16, local_size_z = 4) in;

layout(rgba8) uniform readonly image2D TextureSrc;
layout(rgba32f) uniform coherent image2D TextureDst;

const int offsets[4] = { -2, -1, 1, 2 };

shared vec4 KernelsVal[4];

void main()
{
	ivec2 textureSize = ivec2(imageSize(TextureSrc).xy);
	if (any(greaterThanEqual(gl_GlobalInvocationID.xy, textureSize))) {
		return;
	}

	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	int kernel = int(gl_GlobalInvocationID.z);

	//vec4 val = imageLoad(TextureDst, ivec2(pixel_coords.x + offsets[kernel], pixel_coords.y));
	//if(kernel == 0) val = imageLoad(TextureDst, pixel_coords);
	//val += imageLoad(TextureSrc, pixel_coords);

	KernelsVal[kernel] = imageLoad(TextureSrc, pixel_coords);

	memoryBarrierShared();

	if(kernel == 3)
	{
		imageStore(TextureDst, pixel_coords, (KernelsVal[0] + KernelsVal[1] + KernelsVal[2] + KernelsVal[3]) / 4.0f);
	}
}