vec3 PackNormal(vec3 N)
{
	return N * 0.5 + 0.5;
}

vec3 UnpackNormal(vec3 Packed)
{
	return normalize(Packed * 2.0 - 1.0);
}
