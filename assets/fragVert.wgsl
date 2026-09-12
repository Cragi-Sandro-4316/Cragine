struct Vertex {
    position: vec3f,
    color: vec3f,
    normal: vec3f,
    uv: vec2f
};

const CHUNK_VERTEX_COUNT: u32 = 501;
const ATLAS_PAGE_SIZE: f32 = 128;

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
    pageWidth: f32,
    pageHeight: f32,
    width: u32,
    height: u32
};

@group(0) @binding(0) var<storage, read_write> chunk_buffer: array<MeshChunk>;
@group(0) @binding(1) var<storage, read_write> instance_buffer: array<InstanceData>;
@group(0) @binding(2) var<storage, read_write> map_buffer: array<ChunkMap>;

@group(0) @binding(3) var<uniform> camera: Camera;

@group(0) @binding(4) var<storage, read_write> debug_buffer: array<f32>;

@group(0) @binding(5) var texture_sampler: sampler;

@group(0) @binding(6) var texture: texture_2d<f32>;
@group(0) @binding(7) var<storage, read_write> atlas_entries: array<AtlasEntry>;


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

    let textureID = 4;
    let pageCount = u32(8);
    let totalAtlasWidth = f32(pageCount) * ATLAS_PAGE_SIZE;

    let firstPage = atlas_entries[textureID].firstPageIdx;
    let pageWidth = atlas_entries[textureID].pageWidth;
    let pageHeight = atlas_entries[textureID].pageHeight;

    let width = atlas_entries[textureID].width;
    let height = atlas_entries[textureID].height;

    let textureCoord = in.uv * vec2f(
        f32(width),
        f32(height)
    );

    let uvPageX = u32(in.uv.x * f32(pageWidth));
    let uvPageY = u32(in.uv.y * f32(pageHeight));

    let uvLinearPage = (u32(ceil(pageWidth)) * uvPageY) + uvPageX;

    let coordXInPage = (floor(textureCoord.x) + 0.5) % ATLAS_PAGE_SIZE;
    let coordYInPage = (floor(textureCoord.y) + 0.5) % ATLAS_PAGE_SIZE;

    let absolutePage = firstPage + uvLinearPage;

    let uv = vec2f(
        f32(f32(absolutePage % pageCount) * ATLAS_PAGE_SIZE) + coordXInPage,
        f32(absolutePage / pageCount * u32(ATLAS_PAGE_SIZE)) + coordYInPage
    );

    let color = textureSample(texture, texture_sampler, uv / (ATLAS_PAGE_SIZE * f32(pageCount))).rgb;

    let corrected_color = pow(color, vec3f(2.2));

    return vec4f(corrected_color, 1);
}
