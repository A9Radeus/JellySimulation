#define OUTPUT_PATCH_SIZE 16

//cbuffer cbView : register(b0) { matrix viewMatrix; };

cbuffer cbProj : register(b0) { matrix projMatrix; };

struct HSPatchOutput {
  float edges[4] : SV_TessFactor;
  float inside[2] : SV_InsideTessFactor;
};

struct DSControlPoint {
  float4 pos : POSITION;
  float4 worldPos : POSITION1;
};

struct PSInput {
  float4 pos : SV_POSITION;
  float3 worldPos : POSITION1;
  float3 norm : NORMAL;
};

float3 calcBernstein2Basis(float t) {
  const float invT = (1.0f - t);
  const float B3_0 = invT * invT;
  const float B3_1 = 2.0f * invT * t;
  const float B3_2 = t * t;
  return float3(B3_0, B3_1, B3_2);
}

float3 calcBernstein2Value(float3 pt1, float3 pt2, float3 pt3, float t) {
  float3 basis = calcBernstein2Basis(t);

  float xValue = basis.x * pt1.x + basis.y * pt2.x + basis.z * pt3.x;
  float yValue = basis.x * pt1.y + basis.y * pt2.y + basis.z * pt3.y;
  float zValue = basis.x * pt1.z + basis.y * pt2.z + basis.z * pt3.z;

  return float3(xValue, yValue, zValue);
}

float4 getBernsBasis(float t) {
  float invT = 1.0f - t;
  float B3_0 = invT * invT * invT;
  float B3_1 = 3.0f * t * invT * invT;
  float B3_2 = 3.0f * t * t * invT;
  float B3_3 = t * t * t;

  return float4(B3_0, B3_1, B3_2, B3_3);
}

float3 getCubicBeziPoint(
    const OutputPatch<DSControlPoint, OUTPUT_PATCH_SIZE> patch,
    const float4 basisOne, const float4 basisTwo) {
  // float3 sum = float3(0.0f, 0.0f, 0.0f);

  float3 sum =
      basisOne.x * (basisTwo.x * patch[0].pos + basisTwo.y * patch[1].pos +
                    basisTwo.z * patch[2].pos + basisTwo.w * patch[3].pos);
  sum += basisOne.y * (basisTwo.x * patch[4].pos + basisTwo.y * patch[5].pos +
                       basisTwo.z * patch[6].pos + basisTwo.w * patch[7].pos);
  sum += basisOne.z * (basisTwo.x * patch[8].pos + basisTwo.y * patch[9].pos +
                       basisTwo.z * patch[10].pos + basisTwo.w * patch[11].pos);
  sum += basisOne.w * (basisTwo.x * patch[12].pos + basisTwo.y * patch[13].pos +
                       basisTwo.z * patch[14].pos + basisTwo.w * patch[15].pos);
  return sum;
}

float3 deCasteljauBern3(const float3 b[4], const float t) {
  const float3 common = lerp(b[1], b[2], t);

  const float3 leftQuadratic = lerp(lerp(b[0], b[1], t), common, t);
  const float3 rightQuadratic = lerp(common, lerp(b[2], b[3], t), t);

  return lerp(leftQuadratic, rightQuadratic, t).xyz;
}

// An overload that expands b[4] into b_0, ..., b_3
float3 deCasteljauBern3(const float3 b0, const float3 b1, const float3 b2,
                        const float3 b3, const float t) {
  const float3 common = lerp(b1, b2, t);

  const float3 leftQuadratic = lerp(lerp(b0, b1, t), common, t);
  const float3 rightQuadratic = lerp(common, lerp(b2, b3, t), t);

  return lerp(leftQuadratic, rightQuadratic, t).xyz;
}

float3 deCasteljauBern3_Patch(
    const OutputPatch<DSControlPoint, OUTPUT_PATCH_SIZE> patch, float2 params) {
  const float3 uCoords[4] = {
      deCasteljauBern3(patch[0].pos.xyz, patch[1].pos.xyz, patch[2].pos.xyz,
                       patch[3].pos.xyz, params.y),
      deCasteljauBern3(patch[4].pos.xyz, patch[5].pos.xyz, patch[6].pos.xyz,
                       patch[7].pos.xyz, params.y),
      deCasteljauBern3(patch[8].pos.xyz, patch[9].pos.xyz, patch[10].pos.xyz,
                       patch[11].pos.xyz, params.y), /*
       deCasteljauBern3(patch[8].pos.xyz, patch[8].pos.xyz, patch[10].pos.xyz,
                        patch[11].pos.xyz, params.y),*/
      deCasteljauBern3(patch[12].pos.xyz, patch[13].pos.xyz, patch[14].pos.xyz,
                       patch[15].pos.xyz, params.y),
  };

  const float3 res = deCasteljauBern3(uCoords, params.x);

  return res;
}

// Utility version that calculates the point but in world-coords
float3 deCasteljauBern3_Patch_World(
    const OutputPatch<DSControlPoint, OUTPUT_PATCH_SIZE> patch, float2 params) {
  const float3 uCoords[4] = {
      deCasteljauBern3(patch[0].worldPos.xyz, patch[1].worldPos.xyz,
                       patch[2].worldPos.xyz, patch[3].worldPos.xyz, params.y),
      deCasteljauBern3(patch[4].worldPos.xyz, patch[5].worldPos.xyz,
                       patch[6].worldPos.xyz, patch[7].worldPos.xyz, params.y),
      deCasteljauBern3(patch[8].worldPos.xyz, patch[9].worldPos.xyz,
                       patch[10].worldPos.xyz, patch[11].worldPos.xyz,
                       params.y), /*
deCasteljauBern3(patch[8].worldPos.xyz, patch[8].worldPos.xyz,
patch[10].worldPos.xyz, patch[11].worldPos.xyz, params.y),*/
      deCasteljauBern3(patch[12].worldPos.xyz, patch[13].worldPos.xyz,
                       patch[14].worldPos.xyz, patch[15].worldPos.xyz,
                       params.y),
  };

  const float3 res = deCasteljauBern3(uCoords, params.x);

  return res;
}

float3 deCasteljauBern3_Patch_World_Tan1(
    const OutputPatch<DSControlPoint, OUTPUT_PATCH_SIZE> patch, float2 params) {
  const float3 uCoords[4] = {
      deCasteljauBern3(patch[0].worldPos.xyz, patch[1].worldPos.xyz,
                       patch[2].worldPos.xyz, patch[3].worldPos.xyz, params.y),
      deCasteljauBern3(patch[4].worldPos.xyz, patch[5].worldPos.xyz,
                       patch[6].worldPos.xyz, patch[7].worldPos.xyz, params.y),
      deCasteljauBern3(patch[8].worldPos.xyz, patch[9].worldPos.xyz,
                       patch[10].worldPos.xyz, patch[11].worldPos.xyz,
                       params.y), /*
deCasteljauBern3(patch[8].worldPos.xyz, patch[8].worldPos.xyz,
patch[10].worldPos.xyz, patch[11].worldPos.xyz, params.y),*/
      deCasteljauBern3(patch[12].worldPos.xyz, patch[13].worldPos.xyz,
                       patch[14].worldPos.xyz, patch[15].worldPos.xyz,
                       params.y),
  };

  float3 d1 = 3 * (uCoords[1] - uCoords[0]);
  float3 d2 = 3 * (uCoords[2] - uCoords[1]);
  float3 d3 = 3 * (uCoords[3] - uCoords[2]);
  const float3 res = calcBernstein2Value(d1, d2, d3, params.x);

  return res;
}

float3 deCasteljauBern3_Patch_World_Tan2(
    const OutputPatch<DSControlPoint, OUTPUT_PATCH_SIZE> patch, float2 params) {
  const float3 uCoords[4] = {
      deCasteljauBern3(patch[0].worldPos.xyz, patch[4].worldPos.xyz,
                       patch[8].worldPos.xyz, patch[12].worldPos.xyz, params.x),
      deCasteljauBern3(patch[1].worldPos.xyz, patch[5].worldPos.xyz,
                       patch[9].worldPos.xyz, patch[13].worldPos.xyz, params.x),
      deCasteljauBern3(patch[2].worldPos.xyz, patch[6].worldPos.xyz,
                       patch[10].worldPos.xyz, patch[14].worldPos.xyz,
                       params.x),
      deCasteljauBern3(patch[3].worldPos.xyz, patch[7].worldPos.xyz,
                       patch[11].worldPos.xyz, patch[15].worldPos.xyz,
                       params.x),
  };

  float3 d1 = 3 * (uCoords[1] - uCoords[0]);
  float3 d2 = 3 * (uCoords[2] - uCoords[1]);
  float3 d3 = 3 * (uCoords[3] - uCoords[2]);
  const float3 res = calcBernstein2Value(d1, d2, d3, params.y);

  return res;
}

float3 dUBezier(const OutputPatch<DSControlPoint, OUTPUT_PATCH_SIZE> patch,
                const float u, const float v) {
  float3 P[4];
  float3 vCurve[4];
  for (int i = 0; i < 4; ++i) {
    P[0] = patch[i].worldPos.xyz;
    P[1] = patch[4 + i].worldPos.xyz;
    P[2] = patch[8 + i].worldPos.xyz;
    P[3] = patch[12 + i].worldPos.xyz;
    vCurve[i] = deCasteljauBern3(P, v);
  }

  return -3 * (1 - u) * (1 - u) * vCurve[0] +
         (3 * (1 - u) * (1 - u) - 6 * u * (1 - u)) * vCurve[1] +
         (6 * u * (1 - u) - 3 * u * u) * vCurve[2] + 3 * u * u * vCurve[3];
}

float3 dVBezier(const OutputPatch<DSControlPoint, OUTPUT_PATCH_SIZE> patch,
                const float u, const float v) {
  float3 uCurve[4];
  for (int i = 0; i < 4; ++i) {
    // uCurve[i] = deCasteljauBern3(controlPoints + 4 * i, u);
    uCurve[i] = deCasteljauBern3(
        patch[4 * i].worldPos.xyz, patch[4 * i + 1].worldPos.xyz,
        patch[4 * i + 2].worldPos.xyz, patch[4 * i + 3].worldPos.xyz, u);
  }

  return -3 * (1 - v) * (1 - v) * uCurve[0] +
         (3 * (1 - v) * (1 - v) - 6 * v * (1 - v)) * uCurve[1] +
         (6 * v * (1 - v) - 3 * v * v) * uCurve[2] + 3 * v * v * uCurve[3];
}

/*
  TODO: everything
*/

[domain("quad")] PSInput main(
    float2 uv
    : SV_DomainLocation,
      const OutputPatch<DSControlPoint, OUTPUT_PATCH_SIZE> input,
      HSPatchOutput tessFactors) {
  PSInput o;
  const float4 viewPos = float4(deCasteljauBern3_Patch(input, uv), 1.0f);
  o.worldPos = deCasteljauBern3_Patch_World(input, uv);  // todo
  o.pos = mul(projMatrix, viewPos);

  float3 tan1 = deCasteljauBern3_Patch_World_Tan1(input, uv);
  float3 tan2 = deCasteljauBern3_Patch_World_Tan2(input, uv);
  o.norm = normalize(cross(tan1, tan2));

  return o;
}