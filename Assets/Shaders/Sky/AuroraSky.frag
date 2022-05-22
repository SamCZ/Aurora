layout(early_fragment_tests) in;
layout(location = 0) out vec4 FragColor;

in vec3 Normal;

uniform vec2 u_ViewPort;
uniform mat4 u_InvProjection;
uniform mat4 u_InvView;
uniform vec3 u_ViewPos;
uniform vec3 u_LightDir;
uniform float u_Time;

uniform sampler2D _NoiseTex;
uniform samplerCube _NightBox;

vec3 RGB(float r, float g, float b)
{
	return vec3(r / 255.0, g / 255.0, b / 255.0);
}

const mat2 m2 = mat2(0.8,-0.6,0.6,0.8);

float fbm( vec2 p ) {
	float f = 0.0;
	float scale = 5.0;
	f += 0.5000*texture( _NoiseTex, p/scale, -100. ).x; p = m2*p*2.02;
	f += 0.2500*texture( _NoiseTex, p/scale, -100. ).x; p = m2*p*2.03;
	f += 0.1250*texture( _NoiseTex, p/scale, -100. ).x; p = m2*p*2.01;
	f += 0.0625*texture( _NoiseTex, p/scale, -100. ).x;
	return (f/0.9375) * 1.45;
}

void main()
{
	float x = 2.0 * gl_FragCoord.x / u_ViewPort.x - 1.0;
	float y = 2.0 * gl_FragCoord.y / u_ViewPort.y - 1.0;
	vec2 ray_nds = vec2(x, y);
	vec4 ray_clip = vec4(ray_nds, -1.0, 1.0);
	vec4 ray_view = u_InvProjection * ray_clip;
	ray_view = vec4(ray_view.xy, -1.0, 0.0);
	vec3 ray_world = (u_InvView * ray_view).xyz;
	ray_world = normalize(ray_world);

	float lightDot = dot(u_LightDir, vec3(0, 1, 0));
	float lightDotNormalized = (lightDot + 1.0) / 2.0;

	vec4 cloud_color = vec4(0.0);
	vec4 finalColor = cloud_color;

	float sundot = clamp(dot(ray_world,u_LightDir),0.0,1.0);
	float nightdot = clamp(dot(vec3(0, -1, 0), u_LightDir), 0.0, 1.0);
	vec3 blueSky = vec3(0.3, .55, 0.8);
	vec3 redSky = vec3(0.8, 0.8, 0.6);
	//redSky = mix(redSky, blueSky, 1.0 - nightdot);
	vec3 sky = mix(blueSky, redSky, 1.5*pow(sundot, 8.));

	sky = mix(sky, vec3(0.0), nightdot);

	finalColor.rgb =  sky*(1.0-0.8*ray_world.y);

	/*float starsValue = fbm(ray_world.xz * 100.0);
	if (starsValue > 0.8)
	{
		vec3 backStars = starsValue.xxx;
		backStars = mix(backStars, 0.0.xxx, 1.0 - nightdot);
		finalColor.rgb += backStars;
	}*/

	vec3 sun = vec3(0.0);

	sun += 0.1*vec3(0.9, 0.3, 0.9)*pow(sundot, 0.5);
	sun += 0.2*vec3(1., 0.7, 0.7)*pow(sundot, 1.);
	sun += 0.95*vec3(1.)*pow(sundot, 256.);

	finalColor.rgb += mix(sun, texture(_NightBox, normalize(Normal)).rgb, nightdot);

	/*{
		vec3 ro = vec3(u_ViewPos.x + u_Time * 100, u_ViewPos.y - 100000, u_ViewPos.z + u_Time * 100) / 100.0;
		float cloudSpeed = 0.01;
		float cloudFlux = 0.5;
		vec3 cloudColour = mix(vec3(1.0,0.95,1.0), 0.35*redSky,pow(sundot, 2.));
		cloudColour = mix(cloudColour, cloudColour * 0.1, nightdot);
		if(ray_world.y > 0) {
			float fadeout = clamp((10 / length(ray_world.xz - vec2(0, 0))), 0.0, 1.0);

			for(int i = 0; i < 1; i++) {
				vec2 sc = cloudSpeed * 50.*u_Time * ro.xz + ray_world.xz*((1000.0 + (i * 10.0))-ro.y)/ray_world.y;
				finalColor.rgb = mix( finalColor.rgb, cloudColour, 0.5*smoothstep(0.5,0.8,fbm(0.0005*sc+fbm(0.0005*sc+u_Time*cloudFlux))) * 1);
			}
		}
	}*/

	float pp = pow( 1.-max(ray_world.y+0.1,0.0), 8.0);
	if(!isnan(pp)) {
		vec3 bottomColor = mix( finalColor.rgb, 0.9 * vec3(0.9,0.75,0.8), pp);
		finalColor.rgb = mix(finalColor.rgb, bottomColor, 1.0 - nightdot);
	}

	// contrast
	finalColor.rgb = clamp(finalColor.rgb, 0., 1.);
	finalColor.rgb = finalColor.rgb*finalColor.rgb*(3.0-2.0*finalColor.rgb);


	// saturation (amplify colour, subtract grayscale)
	float sat = 0.2;
	finalColor.rgb = finalColor.rgb * (1. + sat) - sat*dot(finalColor.rgb, vec3(0.33));

	// vignette
	//finalColor.rgb = finalColor.rgb * (1.0 - dot(ray_nds, ray_nds) * 0.1);

	finalColor.a = 1.0;
	FragColor = finalColor;

	// Day values
	/*vec3 DayTopColor = RGB(85, 105, 170);
	vec3 DayBottomColor = RGB(205, 200, 252)* 1.3;
	float DayStartBlend = -0.5;
	float DayEndBlend = 1.5;
	vec3 SunColorUp = RGB(255, 255, 255) * 0.8;
	vec3 SunColorDown = RGB(255, 255, 255) * 0.7;
	float SunStartBlend = 1;
	float SunEndBlend = 0.5;
	float SunSize = 10.0; // This value is inversed, the bigger the value is the smallest is the sun

	// NightValues
	vec3 NightTopColor = RGB(0, 10, 51) * 0.7;
	vec3 NightBottomColor = RGB(205, 220, 255) * 0.5;
	float NightStartBlend = -0.5;
	float NightEndBlend = 1.5;

	// Day blending
	vec3 day = mix(DayBottomColor, DayTopColor, smoothstep(DayStartBlend, DayEndBlend, ray_world.y + 0.5));
	float sundot = clamp(dot(ray_world,u_LightDir),0.0,1.0);
	vec3 sunColor = mix(SunColorDown, SunColorUp, 1.0 - smoothstep(SunStartBlend, SunEndBlend, lightDotNormalized));
	vec3 sun = sunColor * pow(sundot, SunSize);

	// NightBlending
	vec3 night = mix(NightBottomColor, NightTopColor, smoothstep(NightStartBlend, NightEndBlend, ray_world.y + 0.5));

	// Day/Night cycle blending
	//float cycleBlendValue = 1.0 - smoothstep(-0.5, -0.3, sunDot);
	float cycleBlendValue = 1.0 - lightDotNormalized;

	vec3 cycleColor = mix(day, night, cycleBlendValue);
	cycleColor = mix(cycleColor + sun, sun, clamp(pow(sundot, SunSize), 0, 1));

	FragColor = vec4(cycleColor, 1.0);

	// contrast
	// You can disable this if you want, just remove or comment this two lines
	FragColor.rgb = clamp(FragColor.rgb, 0., 1.);
	FragColor.rgb = FragColor.rgb*FragColor.rgb*(3.0-2.0*FragColor.rgb);


	// saturation (amplify colour, subtract grayscale)
	// You can disable this if you want, just remove or comment this two lines
	float sat = 0.2;
	FragColor.rgb = FragColor.rgb * (1. + sat) - sat*dot(FragColor.rgb, vec3(0.33));*/
}