static const int LIGHTS_NUM = 1;

//static const float3 ambientColor = float3(0.05f, 0.05f, 0.05f);
static const float3 ambientColor = float3(0.2f, 0.2f, 0.2f);
static const float3 lightColor = float3(1.0f, 1.0f, 1.0f);
// static const float kd = 0.5f, ks = 0.3f, m = 100.0f;
static const float kd = 0.5f, ks = 0.5f, m = 100.0f;
// static const float4 lightPos = float4(-20.0f, 8.0f, 0.0f, 1.0f);

cbuffer cbSurfaceCol : register(b0) { float4 surfaceCol; };

cbuffer cbLightPos : register(b1) { float4 lightPos; };

cbuffer cbCamPos : register(b2) { float4 camPos; };

struct PSInput {
  float4 pos : SV_POSITION;
  //float3 localPos : POSITION0;
  float3 worldPos : POSITION1;
  float3 norm : NORMAL;
};

float4 main(PSInput i) : SV_TARGET {
  float3 viewVec = normalize(camPos.xyz - i.worldPos);
  //normal = normalize(cross(ddx(i.worldPos), ddy(i.worldPos)));
  float3 normal = normalize(i.norm);
  float3 color = ambientColor;

  [unroll] 
  for (int k = 0; k < LIGHTS_NUM; k++) {
    // float3 lightPosition = (lightPos[k]).xyz;
    float3 lightPosition = (lightPos).xyz;
    float3 lightVec = normalize(lightPosition - i.worldPos);
    float3 halfVec = normalize(viewVec + lightVec);
    color += lightColor * surfaceCol.xyz * kd *
             saturate(dot(normal, lightVec));  // diffuse color
    float nh = dot(normal, halfVec);
    nh = saturate(nh);
    nh = pow(nh, m);
    nh *= ks;
    color += lightColor * nh;
  }

  return float4(saturate(color), surfaceCol.a);
  // return saturate(float4(color, 1.0f) + surfaceCol);
}
