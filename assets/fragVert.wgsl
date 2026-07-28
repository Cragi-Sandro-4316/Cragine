struct Vertex {
    position: vec3f,
    color: vec3f,
};

@group(0) @binding(0) var<storage, read_write> vertex_buffer: array<Vertex>;

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec4f,
};

@vertex
fn vs_main(@builtin(vertex_index) vertexIndex: u32) -> VertexOutput {
    var vertex = vertex_buffer[vertexIndex];

    var out: VertexOutput;
    out.position = vec4f(vertex.position, 1);
    out.color = vec4f(vertex.color, 1);

    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return in.color;
}
