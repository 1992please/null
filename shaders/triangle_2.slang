struct VSInput {
  float2 inPosition;
  float3 inColor;
};

struct VSOutput {
  float4 pos : SV_Position;
  float3 color;
};

struct PushConstants {
  float4x4 model;
  float4x4 viewProj;
};

[vk::push_constant] PushConstants pc;

[shader("vertex")]
VSOutput vertMain(VSInput input) {
  VSOutput output;
  output.pos = mul(pc.viewProj, mul(pc.model, float4(input.inPosition, 0.0, 1.0)));
  output.color = input.inColor;
  return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET { return float4(vertIn.color, 1.0); }