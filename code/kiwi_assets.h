#if !defined(KIWI_ASSETS_H)

enum AssetModels
{
    Model_Sphere,
    Model_Statue,
    
    Model_NumElements,
};

enum AssetTextures
{

    Texture_NumElements,
};

struct file_texture
{
    u32 Width;
    u32 Height;
    v2 Min;
    v2 Dim;
    u64 PixelOffset;
};

struct file_mesh
{
    u32 NumVertices;
    i32 TextureId;
    u64 VertexArrayOffset;
};

struct file_model
{
    u32 NumTextures;
    u64 TextureArrayOffset;
    u32 NumMeshes;
    u64 MeshArrayOffset;
};

struct vertex;
struct asset_texture;
struct asset_mesh
{
    asset_texture* Texture;
    u32 NumVertices;
    vertex* Vertices;
};

struct asset_model
{
    u32 NumTextures;
    asset_texture* TextureArray;
    u32 NumMeshes;
    asset_mesh* Meshes;
};

struct asset_texture
{
    v2 MinUV;
    v2 DimUV;
    f32 AspectRatio;
    // NOTE: This is the atlas id if we are a sub img
    u32* Pixels;
};

struct file_header
{
    u64 ModelArrayPos;
    u64 ModelCount;
        
    u64 TextureArrayPos;
    u64 TextureCount;
};

struct assets
{
    // NOTE: In memory, we have our asset arrays layed out next to each other and after that,
    // we have all the data of our assets laid out following the same order as we see them in
    // our array. This struct just holds the pointers to the array so we can easily access
    // our assets as required.

    u64 ModelCount;
    asset_model* Models;

    u64 TextureCount;
    asset_texture* Textures;
};

inline asset_model* GetModel(assets* Assets, u32 Id)
{
    asset_model* Result = 0;

    Assert(Id < Assets->ModelCount);
    Result = Assets->Models + Id;

    return Result;
}

inline asset_texture* GetTexture(assets* Assets, u32 Id)
{
    asset_texture* Result = 0;

    Assert(Id < Assets->TextureCount);
    Result = Assets->Textures + Id;

    return Result;
}

#define KIWI_ASSETS_H
#endif
