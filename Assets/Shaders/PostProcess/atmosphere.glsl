// for 680, 550, 440
const vec3	SeaLevelScatterCoef		 = vec3(5.8, 13.5, 33.1) * pow(10.0, -6);
const vec3	SeaLevelMScatterCoef	 = vec3(21) * pow(10, -6);
const vec3	SeaLevelOzoneScatterCoef = vec3(0);
const float Hr						 = 1 / 7.994e3; // Hr = 8 km
const float Hm						 = 1 / 1.2e3;	// Hm = 1.2 km
const float scatter2ExtinctionRatio	 = 0.9;			// B^s/B^e = 0.9 according to paper, only areosols absorbs incident light

const Length earthRadius		 = 6378 * km; // 6378 setri se osle
const Length atmosphereThickness = 60 * km;	  // 60km atmosphere

// 695500 km
const Length sunRadius = 695500 * km;
// AU = 149597870700 km;
const Length sunDistance		= 1496e5 * km;
const float	 SunAngularRadius	= asin(sunRadius / sunDistance) * pSunLight.discMultiplier;
const float	 SunAngularDiameter = SunAngularRadius * 2;
// https://en.wikipedia.org/wiki/Sunlight
const Illuminance sunIlluminanceConstant = 128e3 * lux; // lux

SolidAngle GetSunSolidAngle(vec3 normal)
{
	return 1 - cos(0.5 * dot(normal, pSunLight.position));
}

vec3 GetPlanetCenter()
{
	// earth center is exactly under cameraposition
	vec3 camPosition = frame.CameraPosition.xyz / frame.CameraPosition.w;
	camPosition.y	 = -earthRadius;
	return camPosition;
}

Sphere GetAtmosphereBoundary()
{
	return Sphere(GetPlanetCenter(), earthRadius + atmosphereThickness);
}

Sphere GetSunSphere()
{
	vec3 camPosition		= frame.CameraPosition.xyz / frame.CameraPosition.w;
	camPosition.y			= 0;
	const vec3 planetCenter = GetPlanetCenter();
	return Sphere(camPosition + normalize(pSunLight.position) * (earthRadius), 150000.0);
}

vec3 GetSunColor(const Ray r)
{
	const float angle = acos(dot(r.dir, normalize(pSunLight.position)));
	if (angle <= SunAngularRadius)
	{
		return pSunLight.color * sunIlluminanceConstant;
	}
	return vec3(0);
}

float GetAltitude(vec3 point)
{
	return distance(point, GetPlanetCenter()) - earthRadius;
}

vec3 GetAirScatteringCoef(float altitude)
{
	return SeaLevelScatterCoef * exp(-altitude * Hr);
}

vec3 GetAirExtinctionCoef(float altitude)
{
	return GetAirScatteringCoef(altitude);
}

vec3 GetAreosolAbsorbtionCoef(float altitude)
{
	return SeaLevelMScatterCoef * exp(-altitude * Hm);
}

vec3 GetAreosolExtinctionCoef(float altitude)
{
	return GetAreosolAbsorbtionCoef(altitude) / scatter2ExtinctionRatio;
}

vec3 AirMass(const Ray r, float rayLen)
{
	const int numSamples	= 10;
	vec3	  samplingPoint = r.origin;
	float	  stepSize		= rayLen / (numSamples - 1);

	vec3 Be = vec3(0, 0, 0);
	for (int i = 0; i < numSamples; ++i)
	{
		const float altitude = GetAltitude(samplingPoint);
		const vec3	BeR		 = GetAirExtinctionCoef(altitude);
		const vec3	BeM		 = GetAreosolExtinctionCoef(altitude);
		Be += stepSize * (BeR + BeM);
		samplingPoint += r.dir * stepSize;
	}

	return Be;
}

vec3 Transmittance(const Ray r, float rayLen)
{
	return exp(-AirMass(r, rayLen));
}

float ReyleighPhaseFunction(float angle)
{
	const float k = 3.0 / (16.0 * PI);
	return k * (1 + pow(angle, 2.0));
}

// Cornette-Shanks
float MiePhaseFunctionCS(float g, float angle)
{
	const float g2	  = pow(g, 2.0);
	const float k	  = 3.0 / (8.0 * PI);
	const float denom = (2 + g2) * pow((1 + g2 - 2 * g * angle), 3.0 / 2.0);
	return (k * ((1 - g2) * (1 + angle * angle)) / denom);
}

// Henyey-Greenstein
float MiePhaseFunctionHG(float g, float angle)
{
	const float g2 = pow(g, 2.0);
	return 1.0 / (4.0 * PI) * ((1 - g2) / pow(1 + g2 - 2 * g * angle, 3.0 / 2.0));
}

// http://www.csroc.org.tw/journal/JOC25-3/JOC25-3-2.pdf
// Xiao-Lei Fan
float MiePhaseFunctionXLF(float g, float angle)
{
	const float g2	   = pow(g, 2.0);
	const float k	   = 3.0 / 2.0;
	const float denom  = (1 + g2 - 2 * g * angle);
	const float result = k * ((1 - g2) / (2 + g2)) * ((1 + angle * angle) / denom) + g * angle;
	return 1.0 / (4.0 * PI) * result;
}

vec3 InScatteredLight(vec3 y, vec3 v, vec3 s)
{
	const float altitude = GetAltitude(y);
	float		Mie		 = MiePhaseFunctionXLF(pSunLight.asymetricFactor, dot(v, s));
	return GetAirExtinctionCoef(GetAltitude(y)) * ReyleighPhaseFunction(dot(v, s)) + GetAreosolExtinctionCoef(GetAltitude(y)) * Mie;
}

// actually return illuminance (lux)
vec3 GetSkyRadiance(const Ray r, float rayLen)
{
	const Sphere sun			 = GetSunSphere();
	const int	 numSamples		 = 16;
	const int	 numLightSamples = 10;
	vec3		 samplingPoint	 = r.origin;
	float		 stepSize		 = rayLen / numSamples;

	vec3 R_airMassAlongRay = vec3(0);
	vec3 M_airMassAlongRay = vec3(0);

	vec3 R_airMassSum = vec3(0);
	vec3 M_airMassSum = vec3(0);
	vec3 inScattered  = vec3(0);

	vec3 radiance = vec3(0, 0, 0);

	for (int i = 0; i < numSamples; ++i)
	{
		const float altitude = GetAltitude(samplingPoint);
		const vec3	BeR		 = GetAirExtinctionCoef(altitude);
		const vec3	BeM		 = GetAreosolExtinctionCoef(altitude);
		R_airMassAlongRay += BeR * stepSize;
		M_airMassAlongRay += BeM * stepSize;

		vec3	  intersect;
		const Ray toSun = Ray(samplingPoint, normalize(sun.center - samplingPoint));
		Intersect(toSun, sun, intersect);


		// if(SunVisibility(samplingPoint))
		//{
		const float lightStepSize	 = intersect.z / numLightSamples;
		vec3		R_AirMassToLight = vec3(0);
		vec3		M_AirMassToLight = vec3(0);
		for (int j = 0; j < numLightSamples; ++j)
		{
			const vec3 ligthSample = samplingPoint + toSun.dir * j * lightStepSize;
			if (ligthSample.y <= 0)
			{
				// R_AirMassToLight = vec3(0);
				// M_AirMassToLight = vec3(0);
				// continue;
			}
			const float altitude = GetAltitude(ligthSample);
			const vec3	BeR		 = GetAirExtinctionCoef(altitude);
			const vec3	BeM		 = GetAreosolExtinctionCoef(altitude);
			R_AirMassToLight += BeR * lightStepSize;
			M_AirMassToLight += BeM * lightStepSize;
		}
		const vec3 tau		   = (R_airMassAlongRay + R_AirMassToLight) + (M_airMassAlongRay + M_AirMassToLight);
		const vec3 attuneation = exp(-tau);
		R_airMassSum += attuneation * BeR;
		M_airMassSum += attuneation * BeM;
		//}
		samplingPoint += r.dir * stepSize;
	}

	const Ray	toSun  = Ray(r.origin, normalize(sun.center - r.origin));
	const float mu	   = dot(r.dir, toSun.dir);
	const float PMie   = MiePhaseFunctionXLF(pSunLight.asymetricFactor, mu);
	const float PMieHG = MiePhaseFunctionHG(pSunLight.asymetricFactor, mu);
	const float PMieCS = MiePhaseFunctionCS(pSunLight.asymetricFactor, mu);
	const float PRay   = ReyleighPhaseFunction(dot(r.dir, toSun.dir));

	return (R_airMassSum * PRay + M_airMassSum * PMieCS) * sunIlluminanceConstant + exp(-(M_airMassAlongRay + R_airMassAlongRay)) * GetSunColor(r);
}