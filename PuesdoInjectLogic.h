#pragma once



ID3D11VertexShader* __vertexShader;
ID3D11PixelShader* __pixelShader;

ID3D11InputLayout* __inputLayout;

ID3D11Buffer* __vertexBuffer;
ID3D11Buffer* __indexBuffer;

ID3D11SamplerState* __samplerState;
ID3D11ShaderResourceView* __textureView;

ID3D11Buffer* __constantBuffer;


void injected_init(ID3D11DeviceContext1* d3d11DeviceContext) {


    // Create Vertex Shader
    ID3DBlob* vsBlob;
    {
        ID3DBlob* shaderCompileErrorsBlob;
        HRESULT hResult = D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, &shaderCompileErrorsBlob);
        if (FAILED(hResult))
        {
            const char* errorString = NULL;
            if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                errorString = "Could not compile shader; file not found";
            else if (shaderCompileErrorsBlob) {
                errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
                shaderCompileErrorsBlob->Release();
            }
            MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
            return 1;
        }

        hResult = d3d11Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &__vertexShader);
        assert(SUCCEEDED(hResult));
    }

    // Create Pixel Shader
    {
        ID3DBlob* psBlob;
        ID3DBlob* shaderCompileErrorsBlob;
        HRESULT hResult = D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, &shaderCompileErrorsBlob);
        if (FAILED(hResult))
        {
            const char* errorString = NULL;
            if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                errorString = "Could not compile shader; file not found";
            else if (shaderCompileErrorsBlob) {
                errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
                shaderCompileErrorsBlob->Release();
            }
            MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
            return 1;
        }

        hResult = d3d11Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &__pixelShader);
        assert(SUCCEEDED(hResult));
        psBlob->Release();
    }

    // Create Input Layout
    {
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
        {
            { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        HRESULT hResult = d3d11Device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &__inputLayout);
        assert(SUCCEEDED(hResult));
        vsBlob->Release();
    }

    // Create Vertex and Index Buffer
    UINT numIndices;
    UINT stride;
    UINT offset;
    {
        float vertexData[] = { // x, y, z
            -0.5f,-0.5f, -0.5f,
            -0.5f,-0.5f,  0.5f,
            -0.5f, 0.5f, -0.5f,
            -0.5f, 0.5f,  0.5f,
            0.5f,-0.5f, -0.5f,
            0.5f,-0.5f,  0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, 0.5f,  0.5f
        };

        uint16_t indices[] = {
            0, 6, 4,
            0, 2, 6,
            0, 3, 2,
            0, 1, 3,
            2, 7, 6,
            2, 3, 7,
            4, 6, 7,
            4, 7, 5,
            0, 4, 5,
            0, 5, 1,
            1, 5, 7,
            1, 7, 3
        };
        stride = 3 * sizeof(float);
        // numVerts = sizeof(vertexData) / stride;
        offset = 0;
        numIndices = sizeof(indices) / sizeof(indices[0]);

        D3D11_BUFFER_DESC vertexBufferDesc = {};
        vertexBufferDesc.ByteWidth = sizeof(vertexData);
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexSubresourceData = { vertexData };

        HRESULT hResult = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &__vertexBuffer);
        assert(SUCCEEDED(hResult));

        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.ByteWidth = sizeof(indices);
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexSubresourceData = { indices };

        hResult = d3d11Device->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &__indexBuffer);
        assert(SUCCEEDED(hResult));
    }

    // Create Sampler State
    {
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.BorderColor[0] = 1.0f;
        samplerDesc.BorderColor[1] = 1.0f;
        samplerDesc.BorderColor[2] = 1.0f;
        samplerDesc.BorderColor[3] = 1.0f;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

        d3d11Device->CreateSamplerState(&samplerDesc, &__samplerState);
    }

    // Create Texture
    {
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = texWidth;
        textureDesc.Height = texHeight;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
        textureSubresourceData.pSysMem = testTextureBytes;
        textureSubresourceData.SysMemPitch = texBytesPerRow;

        ID3D11Texture2D* texture;
        d3d11Device->CreateTexture2D(&textureDesc, &textureSubresourceData, &texture);

        d3d11Device->CreateShaderResourceView(texture, nullptr, &__textureView);
        texture->Release();
    }

    // constants buffer
    {
        D3D11_BUFFER_DESC constantBufferDesc = {};
        // ByteWidth must be a multiple of 16, per the docs
        constantBufferDesc.ByteWidth = sizeof(Constants) + 0xf & 0xfffffff0;
        constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT hResult = d3d11Device->CreateBuffer(&constantBufferDesc, nullptr, &__constantBuffer);
        assert(SUCCEEDED(hResult));
    }
}

void injected_render(ID3D11DeviceContext1* d3d11DeviceContext) {
    // if hasn't run yet, then initialize
    injected_init(d3d11DeviceContext);



    // Calculate view matrix from camera data
    float4x4 viewMat = translationMat(-cameraPos) * rotateYMat(-cameraYaw) * rotateXMat(-cameraPitch);

    // Spin the cube
    float4x4 modelMat = scaleMat(float3{ 0.5f, 0.5f, 0.5f }) * translationMat(float3{ 1, 1, 1 });

    // Calculate model-view-projection matrix to send to shader
    float4x4 modelViewProj = modelMat * viewMat * perspectiveMat;

    // TODO: load texture to draw onto cube??

    // Update constant buffer
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    d3d11DeviceContext->Map(__constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
    Constants* constants = (Constants*)(mappedSubresource.pData);
    constants->modelViewProj = modelViewProj;
    d3d11DeviceContext->Unmap(__constantBuffer, 0);

    d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d11DeviceContext->IASetInputLayout(__inputLayout);

    d3d11DeviceContext->VSSetShader(__vertexShader, nullptr, 0);
    d3d11DeviceContext->PSSetShader(__pixelShader, nullptr, 0);

    d3d11DeviceContext->VSSetConstantBuffers(0, 1, &__constantBuffer);

    d3d11DeviceContext->PSSetShaderResources(0, 1, &__textureView);
    d3d11DeviceContext->PSSetSamplers(0, 1, &__samplerState);

    d3d11DeviceContext->IASetVertexBuffers(0, 1, &__vertexBuffer, &stride, &offset);
    d3d11DeviceContext->IASetIndexBuffer(__indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    d3d11DeviceContext->DrawIndexed(numIndices, 0, 0);

}