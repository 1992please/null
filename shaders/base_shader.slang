#language slang latest

struct Vertex {
  float3 pos;
  float3 color;
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
  float3 color;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
  VSOutput output;
  DrawInfo draw = pc.drawInfos[input.drawIndex];
  Vertex v = draw.vertices[input.vertexId];

  uint instanceIndex = draw.instanceBaseOffset + input.instanceId;
  InstanceData inst = pc.instances[instanceIndex];

  output.pos = mul(pc.globalUniforms->viewProj, mul(inst.modelMatrix, float4(v.pos, 1.0)));
  output.color = v.color * inst.color.rgb;
  return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET { return float4(vertIn.color, 1.0); }
