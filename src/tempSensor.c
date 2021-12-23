#include "tempSensor.h"

const struct CurveDataPoint *GetCurve() {
  // clang-format off
  static const struct CurveDataPoint s_SDNT1608X103F3950_curve[] = {
      {300000, -36.5},
      {200000, -31.0},
      {100000, -19.5},
      {90000, -18.0},
      {80000, -16.0},
      {70000, -14.0},
      {60000, -11.0},
      {50000, -7.5},
      {40000, -3.5},
      {30000, 2.0},
      {20000, 10.5},
      {10000, 25.0},
      {9000, 27.5},
      {8000, 30.0},
      {7000, 33.5},
      {6000, 37.0},
      {5000, 41.5},
      {4000, 46.5},
      {3000, 55.0},
      {2000, 66.0},
      {1000, 87.0},
      {900, 90.0},
      {800, 94.0},
      {700, 99.0},
      {600, 104.0},
      {500, 111.0},
      {400, 114.5},
      {340, 120.0},
      {0, 0},
  };
  // clang-format on
  return s_SDNT1608X103F3950_curve;
}

float Interpolate(float rt) {
  float t;
  const struct CurveDataPoint *e1 = GetCurve();
  const struct CurveDataPoint *e2 = e1;
  do {
    if (rt >= e2->r) break;
    e1 = e2;
    e2++;
    if (e2->r == 0) e2--;
  } while (e1 != e2);
  if (e1 != e2) {
    // log10 interpolation.
    float f = log10f(rt / e2->r) / log10f(e1->r / e2->r);
    t = e2->t - (e2->t - e1->t) * f;
  } else {
    t = e1->t;
  }
  // LOG(LL_INFO, ("rt %.3f e1 %.3f:%.3f e2 %.3f:%.3f t %.3f", rt, e1->r, e1->t,
  //               e2->r, e2->t, t));
  return t;
}