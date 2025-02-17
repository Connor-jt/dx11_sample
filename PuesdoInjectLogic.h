#pragma once


#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <vector>
#include <cctype>
#include <cstdio>

bool has_injected = false;

// config settings stuff
unsigned long long camera_rotation_yaw_offset = 0;
unsigned long long camera_rotation_pit_offset = 0;
unsigned long long camera_rotation_rol_offset = 0;

unsigned long long camera_position_x_offset = 0;
unsigned long long camera_position_y_offset = 0;
unsigned long long camera_position_z_offset = 0;

static char target_window_name[256] = {};

// these should be per object values !!!!!
static float w_scale = 1.0f;

static bool rotation_relative;
static bool position_relative;

static float w_yaw;
static float w_pitch;
static float w_roll;

static float w_x;
static float w_y;
static float w_z;

// cleanup
ID3D11ShaderResourceView* myTexture = 0;


// other stuff


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



void GetTargetProcessScreen(ID3D11DeviceContext* context);
void Uint64Field(const char* label, unsigned long long* variable);
void injected_render(ID3D11DeviceContext1* d3d11DeviceContext, HWND hwnd, float4x4 viewMat, float3* OFFSET_cameraPos, float* OFFSET_cameraYaw, float* OFFSET_cameraPitch) {
    // if hasn't run yet, then initialize
    if (!has_injected)
        injected_init(d3d11DeviceContext, hwnd);

    // test render random cube code
    {
        // Calculate view matrix from camera data
        float3 __camera_position = { 0.0f, 0.0f, 2.0f };
        float __camera_yaw = 0;
        float __camera_pitch = 0;
        float4x4 __viewMat = translationMat(-__camera_position) * rotateYMat(-__camera_yaw) * rotateXMat(-__camera_pitch);

        // Spin the cube
        //float4x4 modelMat = scaleMat(float3{ 0.5f, 0.5f, 0.5f }) * translationMat(float3{ 5, 0, 0 });
        float4x4 __modelMat = translationMat(float3{ 0, -1, -5 });



        // Calculate model-view-projection matrix to send to shader
        float4x4 __modelViewProj = __modelMat * viewMat * __perspectiveMat;

        // TODO: load texture to draw onto cube??

        // Update constant buffer
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        d3d11DeviceContext->Map(__constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        __Constants* constants = (__Constants*)(mappedSubresource.pData);
        constants->modelViewProj = __modelViewProj;
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
    }


    // now draw our injected UI

    // force update our cheat transform values
    camera_rotation_yaw_offset = (UINT64)OFFSET_cameraYaw;
    camera_rotation_pit_offset = (UINT64)OFFSET_cameraPitch;
    camera_rotation_rol_offset = (UINT64)OFFSET_cameraPitch; // since we dont have a roll value yet

    camera_position_x_offset = (UINT64)OFFSET_cameraPos;
    camera_position_y_offset = (UINT64)OFFSET_cameraPos + 4;
    camera_position_z_offset = (UINT64)OFFSET_cameraPos + 8;

    // reload target window thing
    GetTargetProcessScreen(d3d11DeviceContext);

    std::vector<float4x4> views = {};

    float4x4 modelViewProj;
    if (rotation_relative) {

        // validate camera offsets
        float temp_buffer;
        if (!ReadProcessMemory(GetCurrentProcess(), (LPCVOID)camera_rotation_yaw_offset, &temp_buffer, 4, NULL)
        ||  !ReadProcessMemory(GetCurrentProcess(), (LPCVOID)camera_rotation_pit_offset, &temp_buffer, 4, NULL)
        ||  !ReadProcessMemory(GetCurrentProcess(), (LPCVOID)camera_rotation_rol_offset, &temp_buffer, 4, NULL))
            goto data_offset_failed;

        {   // get widget size
            RECT clientRect;
            GetClientRect(GetActiveWindow(), &clientRect);
            float widget_size = 5.0f / (clientRect.right - clientRect.left);
            widget_size *= w_scale;

            if (!position_relative) {
                float3 widget_camera_position = { 0, 0, 0 };
                // NOTE: cheapo rotation logic that doesn't actually work right!!!!
                float4x4 widget_viewMat = translationMat(-widget_camera_position) * rotateYMat(-*(float*)camera_rotation_yaw_offset + w_yaw) * rotateXMat(-*(float*)camera_rotation_pit_offset + w_pitch);
                float4x4 widget_matrix = scaleMat(float3{ widget_size, -widget_size, -widget_size }) * translationMat({ 0, 0, -1.0f });
                modelViewProj = widget_matrix * widget_viewMat * __perspectiveMat;

            } else { // draw into screen space
                if (!ReadProcessMemory(GetCurrentProcess(), (LPCVOID)camera_position_x_offset, &temp_buffer, 4, NULL)
                ||  !ReadProcessMemory(GetCurrentProcess(), (LPCVOID)camera_position_y_offset, &temp_buffer, 4, NULL)
                ||  !ReadProcessMemory(GetCurrentProcess(), (LPCVOID)camera_position_z_offset, &temp_buffer, 4, NULL))
                    goto data_offset_failed;{

                float3 widget_camera_position = { *(float*)camera_position_x_offset, *(float*)camera_position_y_offset, *(float*)camera_position_z_offset };
                float4x4 widget_viewMat = translationMat(-widget_camera_position) * rotateYMat(-*(float*)camera_rotation_yaw_offset) * rotateXMat(-*(float*)camera_rotation_pit_offset);
                float4x4 widget_matrix = scaleMat(float3{ widget_size, -widget_size, -widget_size }) * rotateYMat(w_yaw) * rotateXMat(w_pitch) * translationMat({ w_x, w_y, w_z });
                modelViewProj = widget_matrix * widget_viewMat * __perspectiveMat;

    }}}} else data_offset_failed:
        modelViewProj = nanMat();
    

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::Begin("Sample copy window", NULL, ImGuiWindowFlags_NoInputs);
    //ImGui::Begin("Sample copy window");
    views.push_back(modelViewProj);

    if (myTexture)
         ImGui::Image((ImTextureID)myTexture, ImVec2(512, 512));
    else ImGui::Text("No image loaded!");
    

    views.push_back(modelViewProj);
    ImGui::End();


    ImGui::Begin("Config UIs");
    ImGui::InputText("Target process", target_window_name, 256);
    ImGui::Checkbox("Camera rotation relative?", &rotation_relative);
    if (rotation_relative) {
        Uint64Field("Camera YAW",   &camera_rotation_yaw_offset);
        Uint64Field("Camera PITCH", &camera_rotation_pit_offset);
        Uint64Field("Camera ROLL",  &camera_rotation_rol_offset);

        ImGui::DragFloat("Widget YAW",   &w_yaw,   0.01f);
        ImGui::DragFloat("Widget PITCH", &w_pitch, 0.01f);
        ImGui::DragFloat("Widget ROLL",  &w_roll,  0.01f);
        ImGui::DragFloat("Widget SCALE", &w_scale, 0.025f, 0.01f);

        ImGui::Checkbox("Camera position relative?", &position_relative);
        if (position_relative) {
            Uint64Field("Camera X", &camera_position_x_offset);
            Uint64Field("Camera Y", &camera_position_y_offset);
            Uint64Field("Camera Z", &camera_position_z_offset);

            ImGui::DragFloat("Widget X", &w_x, 0.025f);
            ImGui::DragFloat("Widget Y", &w_y, 0.025f);
            ImGui::DragFloat("Widget Z", &w_z, 0.025f);
        }
    }
    ImGui::End();
    views.push_back(nanMat());

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData(), views);
}


// Custom validation function
bool IsValidHex(const char* str){
    while (*str){
        if (!std::isxdigit(*str))
            return false;
        ++str;
    }
    return true;
}

void Uint64Field(const char* label, unsigned long long* variable){
    static char input[17] = ""; // Buffer for 16 hex digits + null terminator
    sprintf_s(input, "%016llX", *variable); // fill text buffer with value

    // display label, and if changed then apply
    if (ImGui::InputText(label, input, sizeof(input), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase)
    &&  IsValidHex(input))
        *variable = std::strtoull(input, nullptr, 16);
}


ID3D11ShaderResourceView* CreateTextureFromBitmap(BITMAP bmp, ID3D11Device* device, ID3D11DeviceContext* context, std::vector<BYTE> pixels){
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = bmp.bmWidth;
    desc.Height = bmp.bmHeight;
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;



    ID3D11Texture2D* pTexture = nullptr;
    device->CreateTexture2D(&desc, NULL, &pTexture);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    context->Map(pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource); 

    memcpy(mappedResource.pData, &pixels[0], bmp.bmWidthBytes * bmp.bmHeight);

    context->Unmap(pTexture, 0); 

    ID3D11ShaderResourceView* pSRV = nullptr;
    device->CreateShaderResourceView(pTexture, NULL, &pSRV);
    pTexture->Release();

    return pSRV;
}
void GetTargetProcessScreen(ID3D11DeviceContext* context) {
    // cleanup prev thingo
    if (myTexture) {
        myTexture->Release();
        myTexture = 0;}

    if (!target_window_name[0])
        return;

    // Find the window handle of the application you want to capture
    //HWND hwnd = FindWindowW(NULL, L"*Untitled - Notepad");
    HWND hwnd = FindWindowA(NULL, target_window_name);
    if (!hwnd) 
        return;

    //captureScreenMat(hwnd);

    // Get the device context of the window
    HDC hdcWindow = GetDC(hwnd);
    if (!hdcWindow) return;




    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);

    RECT rc;
    GetClientRect(hwnd, &rc);
    float width = rc.right - rc.left;
    float height = rc.bottom - rc.top;

    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, width, height);
    //auto var = CreateDIBSection(hdcWindow);
    SelectObject(hdcMemDC, hbmScreen);

    BitBlt(hdcMemDC, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY);



    BITMAP bmpScreen;
    GetObjectW(hbmScreen, sizeof(BITMAP), &bmpScreen);

    BITMAPINFOHEADER bi = { 0 };
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;  // Negative to indicate top-down bitmap
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    int rowSize = ((24 * width + 31) / 32) * 4;
    //std::vector<BYTE> pixels(rowSize * height);
    int total_bytes = bmpScreen.bmWidthBytes * bmpScreen.bmHeight;
    std::vector<BYTE> pixels(total_bytes);
    GetDIBits(hdcWindow, hbmScreen, 0, height, pixels.data(), reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

    // reset transparency bits
    for (int i = 3; i < total_bytes; i += 4) {
        pixels[i] = 0xff;
        char p = pixels[i-1];
        pixels[i-1] = pixels[i-3];
        pixels[i-3] = p;
    }
    





    myTexture = CreateTextureFromBitmap(bmpScreen, __d3d11Device, context, pixels);

    DeleteObject(hbmScreen); 
    DeleteDC(hdcMemDC); 
    ReleaseDC(hwnd, hdcWindow);



    //HDC hdcMemDC = CreateCompatibleDC(hdcWindow);
    //if (hdcMemDC) {
    //    RECT rc;
    //    if (GetClientRect(hwnd, &rc)) {


    //        // Create a compatible bitmap
    //        HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, rc.right - rc.left, rc.bottom - rc.top);
    //        if (hbmScreen) {
    //            // Select the compatible bitmap into the memory device context
    //            if (SelectObject(hdcMemDC, hbmScreen)) {
    //                // Bit blit the window into the memory device context
    //                if (BitBlt(hdcMemDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcWindow, 0, 0, SRCCOPY)) {
    //                    BITMAP bmpScreen;
    //                    if (GetObjectW(hbmScreen, sizeof(BITMAP), &bmpScreen))
    //                        myTexture = CreateTextureFromBitmap(bmpScreen, __d3d11Device, context);
    //                }
    //            }
    //            DeleteObject(hbmScreen);
    //        }
    //    }
    //    DeleteObject(hdcMemDC);
    //}
    //ReleaseDC(hwnd, hdcWindow);
}