#pragma once
#include <Arduino.h>

struct CurveDataPoint {
	float r;  // Ohm
	float t;  // Celsius
};

const struct CurveDataPoint *GetCurve();
float Interpolate(float rt);