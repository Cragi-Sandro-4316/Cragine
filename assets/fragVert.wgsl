struct Vertex {
    position: vec3f,
    color: vec3f,
    normal: vec3f,
    uv: vec2f
};

const CHUNK_VERTEX_COUNT: u32 = 501;
const ATLAS_PAGE_SIZE: u32 = 128;

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

struct Camera {
    projectionMatrix: mat4x4f
};

struct AtlasEntry {
    firstPageIdx: u32,
    pageCount: u32
};

@group(0) @binding(0) var<storage, read_write> chunk_buffer: array<MeshChunk>;
@group(0) @binding(1) var<storage, read_write> instance_buffer: array<InstanceData>;
@group(0) @binding(2) var<storage, read_write> map_buffer: array<ChunkMap>;

@group(0) @binding(3) var<uniform> camera: Camera;

@group(0) @binding(4) var texture_sampler: sampler;

@group(0) @binding(5) var texture: texture_2d<f32>;
@group(0) @binding(6) var<storage, read_write> atlas_entries: array<AtlasEntry>;


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
    out.position = camera.projectionMatrix * instance.modelMatrix * vec4f(vertex.position, 1);
    out.color = vec4f(vertex.color, 1);
    out.uv = vertex.uv;

    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    let textureID = 0;

    let firstPage = atlas_entries[textureID].firstPageIdx;
    let pageCount = atlas_entries[textureID].pageCount;



    let color = textureSample(texture, texture_sampler, in.uv).rgb;

    let corrected_color = pow(color, vec3f(2.2));

    return vec4f(corrected_color, 1);
}
