layout(local_size_x = 32, local_size_y = 32) in;

layout(rgba32f) uniform writeonly image2D TextureSrc;

void main()
{
	ivec2 textureSize = ivec2(imageSize(TextureSrc).xy);
	if (any(greaterThanEqual(gl_GlobalInvocationID.xy, textureSize))) {
		return;
	}

	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	imageStore(TextureSrc, pixel_coords, vec4(0, 0, 0, 0));
}