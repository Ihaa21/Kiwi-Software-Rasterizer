internal assets LoadAssets(game_memory* GameMem, mem_arena* Arena)
{
    assets Assets = {};
    file_header* Header = (file_header*)GameMem->AssetMem;
    Assets.ModelCount = Header->ModelCount;
    Assets.TextureCount = Header->TextureCount;
    
    Assets.Models = PushArray(Arena, asset_model, Assets.ModelCount);
    Assets.Textures = PushArray(Arena, asset_texture, Assets.TextureCount);
    
    file_model* FileModels = (file_model*)(GameMem->AssetMem + Header->ModelArrayPos);
    file_texture* FileTextures = (file_texture*)(GameMem->AssetMem + Header->TextureArrayPos);

    for (u64 ModelIndex = 0; ModelIndex < Assets.ModelCount; ++ModelIndex)
    {
        asset_model* Model = Assets.Models + ModelIndex;
        file_model FileModel = FileModels[ModelIndex];

        Model->NumTextures = FileModel.NumTextures;
        if (Model->NumTextures == 0)
        {
            Model->TextureArray = 0;
        }
        else
        {
            Model->TextureArray = PushArray(Arena, asset_texture, FileModel.NumTextures);
        }

        // NOTE: Load the textures
        for (u32 TextureId = 0; TextureId < FileModel.NumTextures; ++TextureId)
        {
            asset_texture* Texture = Model->TextureArray + TextureId;
            file_texture* FileTexture = (file_texture*)(GameMem->AssetMem + FileModel.TextureArrayOffset);
            FileTexture += TextureId;

            Texture->MinUV = V2(0, 0);
            Texture->DimUV = V2(1, 1);
            Texture->AspectRatio = (f32)FileTexture->Width / (f32)FileTexture->Height;

            u32 NumPixels = FileTexture->Width*FileTexture->Height;
            Texture->Pixels = PushArray(Arena, u32, NumPixels);
            Copy((u32*)((u8*)GameMem->AssetMem + FileTexture->PixelOffset), Texture->Pixels,
                 sizeof(u32)*NumPixels);
        }
        
        Model->NumMeshes = FileModel.NumMeshes;
        Model->Meshes = PushArray(Arena, asset_mesh, 1000);

        // NOTE: Load the meshes
        file_mesh* FileMesh = (file_mesh*)(GameMem->AssetMem + FileModel.MeshArrayOffset);
        for (u32 MeshId = 0; MeshId < Model->NumMeshes; ++MeshId)
        {
            asset_mesh* Mesh = Model->Meshes + MeshId;

            Mesh->NumVertices = FileMesh->NumVertices;
            if (FileMesh->NumVertices != 0)
            {
                Mesh->Vertices = PushArray(Arena, vertex, FileMesh->NumVertices);
                Copy((u32*)((u8*)GameMem->AssetMem + FileMesh->VertexArrayOffset), Mesh->Vertices,
                     sizeof(vertex)*FileMesh->NumVertices);
            }

            Mesh->Texture = Model->TextureArray + FileMesh->TextureId;

            FileMesh += 1;
        }
    }

    return Assets;
}
