struct Vertex {
    position: vec4f,
    color: vec4f,
};

@group(0) @binding(0) var<storage, read_write> vertex_buffer: array<Vertex>;

@group(0) @binding(1) var<storage, read_write> index_buffer: array<u32>;

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec4f,
};

@vertex
fn vs_main(@builtin(vertex_index) vertexIndex: u32) -> VertexOutput {

    var index = index_buffer[vertexIndex];

    var vertex = vertex_buffer[index];

    var out: VertexOutput;
    out.position = vertex.position;
    out.color = vertex.color;

    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return in.color;
}
