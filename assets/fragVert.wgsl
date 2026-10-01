struct VertexData {
    position: vec3f,
    color: vec3f,
    normal: vec3f,
    uv: vec2f
};

const CLUSTER_VERTEX_COUNT: u32 = 501;

const MAX_CLUSTER_COUNT: u32 = 2048;
const MAX_INSTANCE_COUNT: u32 = 65535;
const MAX_CLUSTER_INSTANCE_MAP: u32 = 512000;

struct VertexCluster {
    vertexData: array<VertexData, CLUSTER_VERTEX_COUNT>,
    textureIndex: u32
};

struct InstanceData {
    modelMatrix: mat4x4f
};

struct ClusterInstance {
    cluster: u32,
    instance: u32
};

struct MeshCollection {
     clusters: array<VertexCluster, MAX_CLUSTER_COUNT>,
     instances: array<InstanceData, MAX_INSTANCE_COUNT>,
     clusterInstances: array<ClusterInstance, MAX_CLUSTER_INSTANCE_MAP>
};


struct Camera {
    projectionMatrix: mat4x4f
};

const ATLAS_PAGE_SIZE: f32 = 128;
const ATLAS_PAGE_LENGTH: f32 = 8;

struct AtlasEntry {
    firstPageIdx: u32,
    pageWidth: f32,
    pageHeight: f32,
    width: u32,
    height: u32
};

struct AtlasMetadata {
    pageLenght: u32,
    entries: array<AtlasEntry, u32(ATLAS_PAGE_LENGTH * ATLAS_PAGE_LENGTH)>
};

@group(0) @binding(0) var<storage, read_write> mesh_collection: MeshCollection;

@group(0) @binding(1) var<uniform> camera: Camera;

@group(0) @binding(2) var texture_sampler: sampler;

@group(0) @binding(3) var<storage, read_write> atlas_metadata: AtlasMetadata;
@group(0) @binding(4) var texture: texture_2d<f32>;

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec4f,
    @location(1) uv: vec2f,
    @location(2) textureID: u32
};

@vertex
fn vs_main(@builtin(vertex_index) index: u32) -> VertexOutput {

    var mapIndex = u32(index / CLUSTER_VERTEX_COUNT);

    var chunkIndex = mesh_collection.clusterInstances[mapIndex].cluster;

    var instanceIndex = mesh_collection.clusterInstances[mapIndex].instance;

    var vertIdx = index % CLUSTER_VERTEX_COUNT;

    var vertex = mesh_collection.clusters[chunkIndex].vertexData[vertIdx];

    var instance = mesh_collection.instances[instanceIndex];

    var out: VertexOutput;
    out.position = camera.projectionMatrix * instance.modelMatrix * vec4f(vertex.position, 1);
    out.color = vec4f(vertex.color, 1);
    out.uv = vertex.uv;
    out.textureID = mesh_collection.clusters[chunkIndex].textureIndex;

    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {

    let textureID = in.textureID;
    let totalAtlasWidth = f32(atlas_metadata.pageLenght) * ATLAS_PAGE_SIZE;

    let firstPage = atlas_metadata.entries[textureID].firstPageIdx;
    let pageWidth = atlas_metadata.entries[textureID].pageWidth;
    let pageHeight = atlas_metadata.entries[textureID].pageHeight;

    let width = atlas_metadata.entries[textureID].width;
    let height = atlas_metadata.entries[textureID].height;

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
        f32(f32(absolutePage % atlas_metadata.pageLenght) * ATLAS_PAGE_SIZE) + coordXInPage,
        f32(absolutePage / atlas_metadata.pageLenght * u32(ATLAS_PAGE_SIZE)) + coordYInPage
    );

    let color = textureSample(texture, texture_sampler, uv / (ATLAS_PAGE_SIZE * f32(atlas_metadata.pageLenght))).rgb;

    let corrected_color = pow(color, vec3f(2.2));

    return vec4f(corrected_color, 1);
}
