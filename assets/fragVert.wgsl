struct Vertex {
    position: vec3f,
    // color: vec3f,
    normal: vec3f,
    uv: vec2f
};

@group(0) @binding(0) var<storage, read_write> vertex_buffer: array<Vertex>;

// @group(0) @binding(1) var<storage, read_write> index_buffer: array<u32>;

@group(0) @binding(1) var texture_sampler: sampler;

@group(0) @binding(2) var texture: texture_2d<f32>;

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec4f,
    @location(1) uv: vec2f
};

@vertex
fn vs_main(@builtin(vertex_index) index: u32) -> VertexOutput {

    // var index = index_buffer[vertexIndex];

    var vertex = vertex_buffer[index];

    var out: VertexOutput;
    out.position = vec4f(vertex.position, 1);
    // out.color = vec4f(vertex.color, 1);
    out.uv = vertex.uv * 2;

    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {

    let color = textureLoad(texture, vec2i(in.uv), 0).rgb;

    // let color = textureSample(texture, texture_sampler, in.uv).rgb

    let corrected_color = pow(color, vec3f(2.2));

    return vec4f(corrected_color, 1);
}
