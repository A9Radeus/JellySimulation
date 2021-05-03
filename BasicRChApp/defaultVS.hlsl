cbuffer cbWorld : register(b0) { matrix worldMatrix; };

cbuffer cbView : register(b1) {
  matrix viewMatrix;
  matrix invViewMatrix;
};

cbuffer cbProj : register(b2) { matrix projMatrix; };

struct VSInput {
  float3 pos : POSITION;
  //float4 norm : NORMAL;
};

struct PSInput {
  float4 pos : SV_POSITION;
  //float4 norm : NORMAL;
  //float3 localPos : POSITION0;
  //float3 worldPos : POSITION1;
};

PSInput main(VSInput i) {
  PSInput o;
  o.pos = mul(worldMatrix, float4(i.pos, 1.0f));
  o.pos = mul(viewMatrix, o.pos);
  //o.pos = mul(viewMatrix, float4(i.pos, 1.0f));
  o.pos = mul(projMatrix, o.pos);
  //o.norm = i.norm;
  return o;
}