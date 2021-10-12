layout(local_size_x = 32, local_size_y = 32) in;

layout(rgba32f) uniform readonly image2D TextureSrc;
layout(rgba32f) uniform coherent image2D TextureDst;

void main()
{
	ivec2 textureSize = ivec2(imageSize(TextureSrc).xy);
	if (any(greaterThanEqual(gl_GlobalInvocationID.xy, textureSize))) {
		return;
	}

	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

	imageStore(TextureDst, pixel_coords, imageLoad(TextureSrc, pixel_coords) / 5.0f);
}