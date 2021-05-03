cbuffer cbSurfaceCol : register(b0) { float4 surfaceCol; };

cbuffer cbLightPos : register(b1) { float4 lightPos; };

cbuffer cbCamPos : register(b2) { float4 camPos; };

struct PSInput {
  float4 pos : SV_POSITION;
  //float3 worldPos : POSITION1;
  //float3 norm : NORMAL;
   //float3 localPos : POSITION0;
};

float4 main(PSInput psin) : SV_TARGET {
  return surfaceCol;
	//return float4(1.0f, 0.5f, 0.5f, 1.0f);
  //return float4(psin.norm, 1.f);
}