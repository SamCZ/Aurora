layout(rgba16f) writeonly uniform imageCube o_CubeMap;
uniform samplerCube _EnvironmentMap;

vec3 GetCubeMapTexCoord(vec2 cubemapSize)
{
	vec2 st = gl_GlobalInvocationID.xy / cubemapSize;
	vec2 uv = st * 2.0 - 1.0;

	switch(gl_GlobalInvocationID.z)
	{
		case 0: return normalize(vec3(  1.0, uv.y, -uv.x)); // posx
		case 1: return normalize(vec3( -1.0, uv.y,  uv.x)); //negx
		case 2: return normalize(vec3( uv.x,  1.0, -uv.y)); // posy
		case 3: return normalize(vec3( uv.x, -1.0,  uv.y)); //negy
		case 4: return normalize(vec3( uv.x, uv.y,   1.0)); // posz
		case 5: return normalize(vec3(-uv.x, uv.y,  -1.0)); // negz
	}

	return vec3(0.0);
}

const float PI = 3.1415926535897932384626433832795;
const float INV_PI = 1.0 / PI;
const float TWO_PI = PI * 2.0;
const float HALF_PI = PI * 0.5;

const float sampleDelta = 0.025;
const float totalSamples = (TWO_PI / sampleDelta) * (HALF_PI / sampleDelta);
const float invTotalSamples = 1.0 / totalSamples;

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main()
{
	vec2 cubemapSize = vec2(imageSize(o_CubeMap));
	vec3 N = GetCubeMapTexCoord(cubemapSize);

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up = cross(N, right);

	vec3 irradiance = vec3(0.0);

	for(float phi = 0.0; phi < TWO_PI; phi += sampleDelta)
	{
		float sinPhi = sin(phi);
		float cosPhi = cos(phi);

		for(float theta = 0.0; theta < HALF_PI; theta += sampleDelta)
		{
			float sinTheta = sin(theta);
			float cosTheta = cos(theta);

			// spherical to cartesian, in tangent space
			vec3 sphereCoord = vec3(sinTheta * cosPhi,  sinTheta * sinPhi, cosTheta);
			// tangent space to world
			vec3 sampleVec = sphereCoord.x * right + sphereCoord.y * up + sphereCoord.z * N;
			// world to cube coord
			//ivec3 sampleCoord = texCoordToCube(sampleVec, cubemapSize);

			irradiance += texture(_EnvironmentMap, sampleVec).rgb * cosTheta * sinTheta;
		}
	}
	irradiance *= PI * invTotalSamples;

	imageStore(o_CubeMap, ivec3(gl_GlobalInvocationID), vec4(irradiance, 1.0f));
}