inline float FogValue( float a_fDistance )
{
//	return clamp( (a_fDistance - 0.6*g_fGrassRadius) / (g_fGrassRadius * 0.75), 0.0, 0.5);
	float t = clamp((a_fDistance - 0.5*g_fGrassRadius)/ (g_fGrassRadius * 1.), 0.0, 10.0);
//	float t = clamp( (a_fDistance - 0.5*g_fGrassRadius) / (g_fGrassRadius * 0.75), 0.0, 1.0);
	return 1.0 - exp( -t*sqrt(t));
//	return 1.0 - exp( -4.0*t);
}