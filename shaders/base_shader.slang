#language slang latest

struct Vertex {
  float3 pos;
  float3 normal;
  float2 uv;
  float4 color;
};

struct GlobalUniforms {
  float4x4 viewProj;
};

struct DrawInfo {
  Vertex* vertices;
  uint instanceBaseOffset;
  uint padding;
};

struct InstanceData {
  float4x4 modelMatrix;
  float4x4 normalMatrix;
  float4 color;
};

struct PushConstants {
  DrawInfo* drawInfos;
  GlobalUniforms* globalUniforms;
  InstanceData* instances;
};

[vk::push_constant]
PushConstants pc;

struct VSInput {
  uint vertexId : SV_VertexID;
  uint drawIndex : SV_DrawIndex;
  uint instanceId : SV_InstanceID;
};

struct VSOutput {
  float4 pos : SV_Position;
  float3 normal;
  float2 uv;
  float4 color;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
  VSOutput output;
  DrawInfo draw = pc.drawInfos[input.drawIndex];
  Vertex v = draw.vertices[input.vertexId];

  uint instanceIndex = draw.instanceBaseOffset + input.instanceId;
  InstanceData inst = pc.instances[instanceIndex];

  output.pos = mul(pc.globalUniforms->viewProj, mul(inst.modelMatrix, float4(v.pos, 1.0)));
  output.normal = normalize(mul((float3x3)inst.normalMatrix, v.normal));
  output.uv = v.uv;
  output.color = v.color * inst.color;
  return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
  float3 lightDir = normalize(float3(1.0, 0.5, 1.0));
  float3 ambient = float3(0.18, 0.18, 0.18);
  float NdotL = max(dot(normalize(vertIn.normal), lightDir), 0.0);
  float3 lighting = ambient + float3(0.85, 0.85, 0.85) * NdotL;
  return float4(vertIn.color.rgb * lighting, vertIn.color.a);
}
