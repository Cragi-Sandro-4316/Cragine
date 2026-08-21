struct Vertex {
    position: vec3f,
    color: vec3f,
    normal: vec3f,
    uv: vec2f
};

const CHUNK_VERTEX_COUNT: u32 = 501;

struct MeshChunk {
    vertexData: array<Vertex, CHUNK_VERTEX_COUNT>
};

struct InstanceData {
    modelMatrix: mat4x4f
};

@group(0) @binding(0) var<storage, read_write> chunk_buffer: array<MeshChunk>;
@group(0) @binding(1) var<storage, read_write> instance_buffer: array<InstanceData>;
@group(0) @binding(2) var<storage, read_write> map_bufferr: array<i32>;

@group(0) @binding(3) var texture_sampler: sampler;

@group(0) @binding(4) var texture: texture_2d<f32>;

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec4f,
    @location(1) uv: vec2f
};

@vertex
fn vs_main(@builtin(vertex_index) index: u32) -> VertexOutput {

    var chunkIndex = u32(index / CHUNK_VERTEX_COUNT);

    var vertIdx = index - (CHUNK_VERTEX_COUNT * chunkIndex);

    var vertex = chunk_buffer[chunkIndex].vertexData[vertIdx];

    var out: VertexOutput;
    out.position = vec4f(vertex.position, 1);
    out.color = vec4f(vertex.color, 1);
    out.uv = vertex.uv;

    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {

    let color = textureSample(texture, texture_sampler, in.uv).rgb;

    let corrected_color = pow(color, vec3f(2.2));

    return vec4f(corrected_color, 1);
}
