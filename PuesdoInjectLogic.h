#pragma once


#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool has_injected = false;

ID3D11VertexShader* __vertexShader;
ID3D11PixelShader* __pixelShader;

ID3D11InputLayout* __inputLayout;

ID3D11Buffer* __vertexBuffer;
ID3D11Buffer* __indexBuffer;
UINT __numIndices;
UINT __stride;
UINT __offset;

ID3D11SamplerState* __samplerState;
ID3D11ShaderResourceView* __textureView;

ID3D11Buffer* __constantBuffer;

float4x4 __perspectiveMat = {};

ID3D11Device1* __d3d11Device = nullptr;

struct __Constants {
    float4x4 modelViewProj;
};

void injected_init(ID3D11DeviceContext1* d3d11DeviceContext, HWND hwnd) {
    has_injected = true;

    // Get the device
    ID3D11Device* pD3D11Device = nullptr;
    d3d11DeviceContext->GetDevice(&pD3D11Device);
    if (!pD3D11Device) {
        int _3 = 3;
    }
    // transform the device
    // Get the ID3D11Device1 interface
    if (FAILED(pD3D11Device->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&__d3d11Device)))) {
        int _4 = 4;
    }

    // Create Vertex Shader
    ID3DBlob* vsBlob;
    {
        ID3DBlob* shaderCompileErrorsBlob;
        HRESULT hResult = D3DCompileFromFile(L"injected_shaders.hlsl", nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, &shaderCompileErrorsBlob);
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
            return;
        }

        hResult = __d3d11Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &__vertexShader);
        assert(SUCCEEDED(hResult));
    }

    // Create Pixel Shader
    {
        ID3DBlob* psBlob;
        ID3DBlob* shaderCompileErrorsBlob;
        HRESULT hResult = D3DCompileFromFile(L"injected_shaders.hlsl", nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, &shaderCompileErrorsBlob);
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
            return;
        }

        hResult = __d3d11Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &__pixelShader);
        assert(SUCCEEDED(hResult));
        psBlob->Release();
    }

    // Create Input Layout
    {
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0  },
        };

        HRESULT hResult = __d3d11Device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &__inputLayout);
        assert(SUCCEEDED(hResult));
        vsBlob->Release();
    }

    // Create Vertex and Index Buffer
    {
        float vertexData[] = { 
        //  pos_x, pos_y, pos_z,    ux_x, uv_y
            -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,    0.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,    1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,    1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,    1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,    0.0f, 0.0f
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
        __stride = 5 * sizeof(float);
        __offset = 0;
        __numIndices = sizeof(indices) / sizeof(indices[0]);

        D3D11_BUFFER_DESC vertexBufferDesc = {};
        vertexBufferDesc.ByteWidth = sizeof(vertexData);
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexSubresourceData = { vertexData };

        HRESULT hResult = __d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &__vertexBuffer);
        assert(SUCCEEDED(hResult));

        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.ByteWidth = sizeof(indices);
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexSubresourceData = { indices };

        hResult = __d3d11Device->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &__indexBuffer);
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

        __d3d11Device->CreateSamplerState(&samplerDesc, &__samplerState);
    }


    // Load Image
    int texWidth, texHeight, texNumChannels;
    int texForceNumChannels = 4;
    unsigned char* testTextureBytes = stbi_load("test.png", &texWidth, &texHeight,
        &texNumChannels, texForceNumChannels);
    assert(testTextureBytes);
    int texBytesPerRow = 4 * texWidth;

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
        __d3d11Device->CreateTexture2D(&textureDesc, &textureSubresourceData, &texture);

        __d3d11Device->CreateShaderResourceView(texture, nullptr, &__textureView);
        texture->Release();
    }

    // constants buffer
    {
        D3D11_BUFFER_DESC constantBufferDesc = {};
        // ByteWidth must be a multiple of 16, per the docs
        constantBufferDesc.ByteWidth = sizeof(__Constants) + 0xf & 0xfffffff0;
        constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT hResult = __d3d11Device->CreateBuffer(&constantBufferDesc, nullptr, &__constantBuffer);
        assert(SUCCEEDED(hResult));
    }



    // Get window dimensions
    int windowWidth, windowHeight;
    float windowAspectRatio;
    {
        auto var = GetActiveWindow();
        RECT clientRect;
        GetClientRect(var, &clientRect);
        windowWidth = clientRect.right - clientRect.left;
        windowHeight = clientRect.bottom - clientRect.top;
        windowAspectRatio = (float)windowWidth / (float)windowHeight;
    }
    __perspectiveMat = makePerspectiveMat(windowAspectRatio, degreesToRadians(84), 0.1f, 1000.f);



    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(__d3d11Device, d3d11DeviceContext);
}

void injected_render(ID3D11DeviceContext1* d3d11DeviceContext, HWND hwnd) {
    // if hasn't run yet, then initialize
    if (!has_injected) 
        injected_init(d3d11DeviceContext, hwnd);



    // Calculate view matrix from camera data
    float3 __camera_position = {0.0f, 0.0f, 2.0f};
    float __camera_yaw = 0;
    float __camera_pitch = 0;
    float4x4 viewMat = translationMat(-__camera_position) * rotateYMat(-__camera_yaw) * rotateXMat(-__camera_pitch);

    // Spin the cube
    //float4x4 modelMat = scaleMat(float3{ 0.5f, 0.5f, 0.5f }) * translationMat(float3{ 5, 0, 0 });
    float4x4 modelMat = translationMat(float3{ 0, 0, -5 });

    // Calculate model-view-projection matrix to send to shader
    float4x4 modelViewProj = modelMat * viewMat * __perspectiveMat;

    // TODO: load texture to draw onto cube??

    // Update constant buffer
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    d3d11DeviceContext->Map(__constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
    __Constants* constants = (__Constants*)(mappedSubresource.pData);
    constants->modelViewProj = modelViewProj;
    d3d11DeviceContext->Unmap(__constantBuffer, 0);

    d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d11DeviceContext->IASetInputLayout(__inputLayout);

    d3d11DeviceContext->VSSetShader(__vertexShader, nullptr, 0);
    d3d11DeviceContext->PSSetShader(__pixelShader, nullptr, 0);

    d3d11DeviceContext->VSSetConstantBuffers(0, 1, &__constantBuffer);

    d3d11DeviceContext->PSSetShaderResources(0, 1, &__textureView);
    d3d11DeviceContext->PSSetSamplers(0, 1, &__samplerState);

    d3d11DeviceContext->IASetVertexBuffers(0, 1, &__vertexBuffer, &__stride, &__offset);
    d3d11DeviceContext->IASetIndexBuffer(__indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    d3d11DeviceContext->DrawIndexed(__numIndices, 0, 0);




    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Another Window");
    ImGui::Text("Hello from another window!");
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}