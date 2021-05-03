#define INPUT_PATCH_SIZE 16
#define OUTPUT_PATCH_SIZE 16

cbuffer cbView : register(b0) {
  matrix viewMtx;
};

cbuffer cbTessFacts : register(b1) {
  // For now only uses 2 coords:
  // .x -- col-slice factor
  // .y -- row-slice factor
  // .z -- inside factor
  float4 tessFacts; 
};

struct HSInput {
  float4 pos : POSITION;
};

struct HSPatchOutput {
  float edges[4] : SV_TessFactor;
  float inside[2] : SV_InsideTessFactor;
};

struct DSControlPoint {
  float4 pos : POSITION;
  float4 worldPos : POSITION1;
};

HSPatchOutput HS_Patch(InputPatch<HSInput, INPUT_PATCH_SIZE> ip,
                       uint patchId : SV_PrimitiveID) {
  HSPatchOutput o;
  //o.edges[0] = tessFacts.x;
  //o.edges[1] = tessFacts.x;
  //o.edges[2] = tessFacts.x;
  //o.edges[3] = tessFacts.x;
  //o.inside[0] = tessFacts.y;
  //o.inside[1] = tessFacts.y;
  o.edges[0] = tessFacts.y;
  o.edges[1] = tessFacts.x;
  o.edges[2] = tessFacts.y;
  o.edges[3] = tessFacts.x;
  o.inside[0] = tessFacts.z;
  o.inside[1] = tessFacts.z;
  
  return o;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(OUTPUT_PATCH_SIZE)]
[patchconstantfunc("HS_Patch")]
DSControlPoint main(
  InputPatch<HSInput, INPUT_PATCH_SIZE> ip,
  uint i : SV_OutputControlPointID,
  uint patchID : SV_PrimitiveID) 
{
  DSControlPoint o;
  o.worldPos = ip[i].pos;
  o.pos = mul(viewMtx, ip[i].pos);
  return o;
}