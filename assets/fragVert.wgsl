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

struct ChunkMap {
    chunk: u32,
    instance: u32
};

@group(0) @binding(0) var<storage, read_write> chunk_buffer: array<MeshChunk>;
@group(0) @binding(1) var<storage, read_write> instance_buffer: array<InstanceData>;
@group(0) @binding(2) var<storage, read_write> map_buffer: array<ChunkMap>;

@group(0) @binding(3) var<storage, read_write> debug_buffer: array<u32>;

@group(0) @binding(4) var texture_sampler: sampler;

@group(0) @binding(5) var texture: texture_2d<f32>;

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec4f,
    @location(1) uv: vec2f
};

@vertex
fn vs_main(@builtin(vertex_index) index: u32) -> VertexOutput {

    var mapIndex = u32(index / CHUNK_VERTEX_COUNT);

    var chunkIndex = map_buffer[mapIndex].chunk;

    var instanceIndex = map_buffer[mapIndex].instance;

    var vertIdx = index % CHUNK_VERTEX_COUNT;

    var vertex = chunk_buffer[chunkIndex].vertexData[vertIdx];

    var instance = instance_buffer[instanceIndex];

    var out: VertexOutput;
    out.position = instance.modelMatrix * vec4f(vertex.position, 1);
    out.color = vec4f(vertex.color, 1);
    out.uv = vertex.uv;


    if index < 501 {
        debug_buffer[0] = mapIndex;
        debug_buffer[1] = chunkIndex;
        debug_buffer[2] = instanceIndex;
        debug_buffer[3] = 67;
    }
    else {
        debug_buffer[4] = mapIndex;
        debug_buffer[5] = chunkIndex;
        debug_buffer[6] = instanceIndex;
        debug_buffer[7] = 67;
    }

    debug_buffer[8] = 67;
    debug_buffer[9] = 67;


    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {

    let color = textureSample(texture, texture_sampler, in.uv).rgb;

    let corrected_color = pow(color, vec3f(2.2));

    return vec4f(corrected_color, 1);
}
