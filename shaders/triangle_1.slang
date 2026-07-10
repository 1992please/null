struct Vertex {
  float2 pos;
  float3 color;
};

struct PushConstants {
  Vertex* vertices;
};

[vk::push_constant] PushConstants pc;

struct VSOutput {
  float4 pos : SV_Position;
  float3 color;
};

[shader("vertex")]
VSOutput vertMain(uint vid : SV_VertexID) {
  VSOutput output;
  Vertex v = pc.vertices[vid];
  output.pos = float4(v.pos, 0.0, 1.0);
  output.color = v.color;
  return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET { return float4(vertIn.color, 1.0); }