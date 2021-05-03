
cbuffer cbView : register(b0) {
  matrix viewMtx;
  matrix invViewMtx;
};

cbuffer cbProj : register(b1) { matrix projMtx; };

cbuffer cbCamPos : register(b2) { float4 rightDir; };
// cbuffer cbCamPos : register(b2) { float3 camPos; };

struct VSOutput {
  float4 worldPos : POSITION;
};

struct GSOutput {
  float4 posH : SV_POSITION;
};

[maxvertexcount(4)] void main(point VSOutput input[1], uint primID
                              : SV_PrimitiveID,
                                inout TriangleStream<GSOutput> output) {
  // --- Pos-s
  const float3 pointPos = input[0].worldPos.xyz;
  const float3 camPos = mul(invViewMtx, float4(0.0f, 0.0f, 0.0f, 1.0f));

  // --- Dirs
  const float3 rDir = normalize(-rightDir.xyz);  // Camera's rDir ~= My lDir
  const float3 camDir = normalize(pointPos - camPos);
  const float3 upDir = normalize(cross(camDir, rDir));

  // --- Point's size
  const float camDistance =
      distance(pointPos, camPos) + 0.00001f;  // to make it != 0
  const float halfW = 0.005f * (camDistance);
  const float halfH = 0.005f * (camDistance);

  float4 vBuff[4];
  vBuff[0] = float4(pointPos + (halfW * rDir) - (halfH * upDir), 1.0f);
  vBuff[1] = float4(pointPos + (halfW * rDir) + (halfH * upDir), 1.0f);
  vBuff[2] = float4(pointPos - (halfW * rDir) - (halfH * upDir), 1.0f);
  vBuff[3] = float4(pointPos - (halfW * rDir) + (halfH * upDir), 1.0f);

  GSOutput gsOut;
  [unroll] for (int i = 0; i < 4; ++i) {
    gsOut.posH = mul(viewMtx, vBuff[i]);
    gsOut.posH = mul(projMtx, gsOut.posH);
    // gsOut.col = float4(1.0f, 1.0f, 0.0f, 1.0f);
    // gsOut.NormalW = look;
    // gsOut.Tex = gTexC[i];
    // gsOut.PrimID = primID;
    output.Append(gsOut);
  }
}