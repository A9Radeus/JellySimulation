cbuffer cbModel : register(b0) { matrix modelMtx; };

struct VSInput {
  float4 pos : POSITION;
};

struct VSOutput {
  float4 worldPos : POSITION;
};

VSOutput main(VSInput i) {
  VSOutput o;
  o.worldPos = mul(modelMtx, i.pos);
  return o;
}