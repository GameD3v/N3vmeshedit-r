#include "D3D11Renderer.h"
#include <cstdio>       // fprintf için
#include <exception>    // std::exception için
#include <d3dcompiler.h> // D3DCompile fonksiyonu ve bayrakları için
#pragma comment(lib, "d3dcompiler.lib") // D3DCompile fonksiyonu için gerekli kütüphaneyi otomatik bağlar.

// DirectXMath kütüphanesini kullanmak için bu namespace'i import ediyoruz
using namespace DirectX;

// Basic Vertex Shader HLSL kodu
const char* g_VertexShader = R"(
cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
};

struct VS_INPUT
{
    float3 Pos : POSITION;
    float4 Color : COLOR;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    // Dünya, Görünüm ve Projeksiyon matrislerini uygulayarak vertex pozisyonunu dönüştür
    output.Pos = mul(float4(input.Pos, 1.0f), World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.Color = input.Color; // Rengi doğrudan geçir
    return output;
}
)";

// Basic Pixel Shader HLSL kodu
const char* g_PixelShader = R"(
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

float4 PSMain(PS_INPUT input) : SV_Target
{
    return input.Color; // Gelen rengi doğrudan çıktı olarak ver
}
)";

// Sabit tampon yapısı (matrisleri shader'a göndermek için)
struct ConstantBufferData
{
    XMMATRIX World;
    XMMATRIX View;
    XMMATRIX Projection;
};


D3D11Renderer::D3D11Renderer()
    : m_windowWidth(0)
    , m_windowHeight(0)
    , m_vertexCount(0)
    , m_indexCount(0)
    , m_cameraPos(0.0f, 0.0f, -5.0f) // Başlangıç kamera pozisyonu
    , m_cameraTarget(0.0f, 0.0f, 0.0f) // Kameranın baktığı hedef
    , m_cameraUp(0.0f, 1.0f, 0.0f) // Kameranın yukarı yönü
{
    // Matrisleri kimlik matrisine (identity matrix) ayarla
    m_worldMatrix = XMMatrixIdentity();
    m_viewMatrix = XMMatrixIdentity();
    m_projectionMatrix = XMMatrixIdentity();
}

D3D11Renderer::~D3D11Renderer()
{
    Shutdown(); // Kaynakları serbest bırak
}

// Direct3D'yi başlatma
bool D3D11Renderer::Initialize(HWND hWnd, int width, int height)
{
    m_windowWidth = width;
    m_windowHeight = height;

    if (!CreateDeviceAndSwapChain(hWnd, width, height))
        return false;

    if (!CreateShadersAndInputLayout())
        return false;

    if (!CreateConstantBuffer())
        return false;

    SetupCamera(); // Kamerayı ayarla
    UpdateMatrices(); // Matrisleri güncelle

    return true;
}

// Direct3D kaynaklarını temizleme
void D3D11Renderer::Shutdown()
{
    // ComPtr'lar otomatik olarak Release() çağırır, bu yüzden manuel olarak Release yapmamıza gerek yok.
    m_d3dDevice.Reset();
    m_d3dContext.Reset();
    m_swapChain.Reset();
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();

    m_vertexShader.Reset();
    m_pixelShader.Reset();
    m_inputLayout.Reset();
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
    m_constantBuffer.Reset();

    m_vertexCount = 0;
    m_indexCount = 0;
}

// Sahneyi çizme
void D3D11Renderer::Render()
{
    if (!m_d3dContext || !m_renderTargetView) return;

    // Sahneyi temizle (mavi arka plan rengiyle)
    float clearColor[4] = { 0.1f, 0.2f, 0.3f, 1.0f }; // Açık mavi
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Eğer çizilecek bir mesh varsa
    if (m_vertexCount > 0 && m_indexCount > 0)
    {
        // Matrisleri shader'a kopyala
        ConstantBufferData cb;
        cb.World = XMMatrixTranspose(m_worldMatrix); // HLSL, matrisleri transpoze olarak bekler
        cb.View = XMMatrixTranspose(m_viewMatrix);
        cb.Projection = XMMatrixTranspose(m_projectionMatrix);
        m_d3dContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

        // Input Assembler (IA) aşaması ayarları
        UINT stride = sizeof(VertexPositionColor); // Bir vertex'in boyutu
        UINT offset = 0;
        m_d3dContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
        m_d3dContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0); // WORD = R16_UINT
        m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // Üçgen listesi olarak çiz

        // Shader ayarları
        m_d3dContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
        m_d3dContext->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
        m_d3dContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);
        m_d3dContext->IASetInputLayout(m_inputLayout.Get());

        // Çizim komutu
        m_d3dContext->DrawIndexed(m_indexCount, 0, 0);
    }

    // Görüntüyü ekrana sun
    m_swapChain->Present(1, 0); // 1 = VSync bekle
}

// Pencere boyutu değiştiğinde çağrılır
void D3D11Renderer::Resize(int width, int height)
{
    if (width == m_windowWidth && height == m_windowHeight) return;

    m_windowWidth = width;
    m_windowHeight = height;

    if (m_d3dContext)
    {
        // Render hedef ve derinlik/stencil görünümlerini serbest bırak
        m_renderTargetView.Reset();
        m_depthStencilView.Reset();

        // Swap chain'i yeniden boyutlandır
        HRESULT hr = m_swapChain->ResizeBuffers(0, m_windowWidth, m_windowHeight, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(hr))
        {
            fprintf(stderr, "Hata: Swap Chain yeniden boyutlandirilamadi! HRESULT: %08X\n", hr);
            return;
        }

        // Yeniden boyutlandırılmış back buffer'ı al ve yeni bir render hedef görünümü oluştur
        ID3D11Texture2D* backBuffer;
        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
        if (FAILED(hr)) {
            fprintf(stderr, "Hata: Back buffer alinirken hata! HRESULT: %08X\n", hr);
            return;
        }
        hr = m_d3dDevice->CreateRenderTargetView(backBuffer, nullptr, m_renderTargetView.GetAddressOf());
        backBuffer->Release(); // Back buffer'ı Release etmeyi unutma!
        if (FAILED(hr)) {
            fprintf(stderr, "Hata: Render Target View olusturulurken hata! HRESULT: %08X\n", hr);
            return;
        }

        // Yeni derinlik/stencil arabelleğini ve görünümünü oluştur
        D3D11_TEXTURE2D_DESC depthStencilDesc;
        depthStencilDesc.Width = m_windowWidth;
        depthStencilDesc.Height = m_windowHeight;
        depthStencilDesc.MipLevels = 1;
        depthStencilDesc.ArraySize = 1;
        depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24-bit derinlik, 8-bit stencil
        depthStencilDesc.SampleDesc.Count = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
        depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
        depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilDesc.CPUAccessFlags = 0;
        depthStencilDesc.MiscFlags = 0;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
        hr = m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencilBuffer.GetAddressOf());
        if (FAILED(hr)) {
            fprintf(stderr, "Hata: Depth Stencil Buffer olusturulurken hata! HRESULT: %08X\n", hr);
            return;
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
        ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
        depthStencilViewDesc.Format = depthStencilDesc.Format;
        depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Texture2D.MipSlice = 0;

        hr = m_d3dDevice->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, m_depthStencilView.GetAddressOf());
        if (FAILED(hr)) {
            fprintf(stderr, "Hata: Depth Stencil View olusturulurken hata! HRESULT: %08X\n", hr);
            return;
        }

        // Render hedeflerini (back buffer ve derinlik/stencil) çıktı birleştirici (OM) aşamasına ayarla
        m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Viewport'u (görünüm alanı) ayarla
        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)m_windowWidth;
        vp.Height = (FLOAT)m_windowHeight;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_d3dContext->RSSetViewports(1, &vp);

        // Projeksiyon matrisini de yeniden hesapla (boyut değiştiği için)
        UpdateMatrices();
    }
}

// Mesh verilerini Direct3D buffer'larına yükle
void D3D11Renderer::LoadMesh(const CN3VMesh& mesh)
{
    if (mesh.GetVertexCount() == 0 || mesh.GetIndexCount() == 0)
    {
        fprintf(stderr, "Hata: Yuklenecek mesh verisi bulunamadi (vertex veya index sayisi sifir).\n");
        m_vertexCount = 0;
        m_indexCount = 0;
        m_vertexBuffer.Reset();
        m_indexBuffer.Reset();
        return;
    }

    m_vertexCount = mesh.GetVertexCount();
    m_indexCount = mesh.GetIndexCount();

    // Vertex buffer oluşturma
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(VertexPositionColor) * m_vertexCount;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = mesh.GetVertices().data(); // CN3VMesh'ten verileri al
    HRESULT hr = m_d3dDevice->CreateBuffer(&bd, &InitData, m_vertexBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        fprintf(stderr, "Hata: Vertex Buffer olusturulurken hata! HRESULT: %08X\n", hr);
        return;
    }

    // Index buffer oluşturma
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(unsigned short) * m_indexCount; // unsigned short = WORD
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = mesh.GetIndices().data(); // CN3VMesh'ten verileri al
    hr = m_d3dDevice->CreateBuffer(&bd, &InitData, m_indexBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        fprintf(stderr, "Hata: Index Buffer olusturulurken hata! HRESULT: %08X\n", hr);
        return;
    }

    // Mesh yüklendiğinde kamerayı mesh'in merkezine odakla
    // CN3VMesh'deki GetCenter() ve GetRadius() kullanılıyor.
    m_cameraTarget = XMFLOAT3(mesh.GetCenter().x, mesh.GetCenter().y, mesh.GetCenter().z);

    // Kamerayı mesh'in biraz gerisine yerleştir
    float cameraDistance = mesh.GetRadius() * 2.5f; // Yarıçapın 2.5 katı kadar geride
    if (cameraDistance < 5.0f) cameraDistance = 5.0f; // Minimum mesafe
    m_cameraPos = XMFLOAT3(m_cameraTarget.x, m_cameraTarget.y, m_cameraTarget.z - cameraDistance);

    SetupCamera(); // Kamera pozisyonunu güncelle
    UpdateMatrices(); // Matrisleri yeniden hesapla
}

// Mesh'i belirli bir öteleme miktarıyla hareket ettirir
void D3D11Renderer::TranslateMesh(float x, float y, float z)
{
    // Mevcut dünya matrisini al ve öteleme uygulayarak yeni bir matris oluştur
    XMMATRIX translationMatrix = XMMatrixTranslation(x, y, z);
    m_worldMatrix = XMMatrixMultiply(m_worldMatrix, translationMatrix); // Mevcut dünya matrisini ötele
}


// Direct3D Cihazı ve Swap Chain oluşturma
bool D3D11Renderer::CreateDeviceAndSwapChain(HWND hWnd, int width, int height)
{
    // Device ve Swap Chain oluşturma bayrakları
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG; // Debug katmanlarını etkinleştir
#endif

    // Özellik seviyeleri (Feature Levels) - Hangi DirectX versiyonlarını destekleyeceğimizi belirtiriz
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    // Swap chain açıklaması
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;                                 // Tek bir back buffer
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // 32-bit renk formatı
    sd.BufferDesc.RefreshRate.Numerator = 60;           // Yenileme hızı 60 Hz
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;   // Render hedefi olarak kullanılacak
    sd.OutputWindow = hWnd;                             // Çizim yapılacak pencere
    sd.SampleDesc.Count = 1;                            // Çoklu örnekleme yok (MSAA)
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;                                 // Pencereli mod
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;           // Eski back buffer'ı at

    // Cihazı, cihaz bağlamını ve swap chain'i oluşturma
    D3D_FEATURE_LEVEL actualFeatureLevel; // Oluşturulan cihazın gerçek özellik seviyesi
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        createDeviceFlags, featureLevels, numFeatureLevels,
        D3D11_SDK_VERSION, &sd, m_swapChain.GetAddressOf(),
        m_d3dDevice.GetAddressOf(), &actualFeatureLevel, m_d3dContext.GetAddressOf());

    if (FAILED(hr))
    {
        fprintf(stderr, "Hata: Direct3D 11 Cihaz ve Swap Chain olusturulurken hata! HRESULT: %08X\n", hr);
        return false;
    }

    // Back buffer'ı al ve render hedef görünümü oluştur
    ID3D11Texture2D* backBuffer;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    if (FAILED(hr)) {
        fprintf(stderr, "Hata: Back buffer alinirken hata! HRESULT: %08X\n", hr);
        return false;
    }
    hr = m_d3dDevice->CreateRenderTargetView(backBuffer, nullptr, m_renderTargetView.GetAddressOf());
    backBuffer->Release(); // Back buffer'ı Release etmeyi unutma!
    if (FAILED(hr)) {
        fprintf(stderr, "Hata: Render Target View olusturulurken hata! HRESULT: %08X\n", hr);
        return false;
    }

    // Derinlik/stencil arabelleğini ve görünümünü oluştur
    D3D11_TEXTURE2D_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
    depthStencilDesc.Width = width;
    depthStencilDesc.Height = height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24-bit derinlik, 8-bit stencil
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
    hr = m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencilBuffer.GetAddressOf());
    if (FAILED(hr)) {
        fprintf(stderr, "Hata: Depth Stencil Buffer olusturulurken hata! HRESULT: %08X\n", hr);
        return false;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
    depthStencilViewDesc.Format = depthStencilDesc.Format;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    hr = m_d3dDevice->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, m_depthStencilView.GetAddressOf());
    if (FAILED(hr)) {
        fprintf(stderr, "Hata: Depth Stencil View olusturulurken hata! HRESULT: %08X\n", hr);
        return false;
    }

    // Render hedeflerini (back buffer ve derinlik/stencil) çıktı birleştirici (OM) aşamasına ayarla
    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Viewport'u (görünüm alanı) ayarla
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_d3dContext->RSSetViewports(1, &vp);

    return true;
}

// Shader'ları (HLSL kodlarını) derleme ve Input Layout oluşturma
bool D3D11Renderer::CreateShadersAndInputLayout()
{
    HRESULT hr;

    // Vertex Shader derleme
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

    // D3DCompile çağrısı düzeltildi: 10 parametre bekleniyor.
    // pSourceName ve pInclude parametreleri nullptr olarak geçirildi.
    hr = D3DCompile(g_VertexShader,           // pSrcData
        strlen(g_VertexShader),   // SrcDataSize
        nullptr,                  // pSourceName (opsiyonel)
        nullptr,                  // pDefines (opsiyonel)
        nullptr,                  // pInclude (opsiyonel)
        "VSMain",                 // pEntrypoint
        "vs_5_0",                 // pTarget
        D3DCOMPILE_ENABLE_STRICTNESS, // Flags1
        0,                        // Flags2
        vsBlob.GetAddressOf(),    // ppCode
        errorBlob.GetAddressOf()  // ppErrorMsgs (opsiyonel)
    );

    if (FAILED(hr))
    {
        if (errorBlob) fprintf(stderr, "Vertex Shader Derleme Hatasi: %s\n", (char*)errorBlob->GetBufferPointer());
        return false;
    }
    hr = m_d3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
    if (FAILED(hr)) {
        fprintf(stderr, "Hata: Vertex Shader olusturulurken hata! HRESULT: %08X\n", hr);
        return false;
    }

    // Input Layout açıklaması (Vertex yapımızla eşleşmeli)
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // Position (float3)
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // Color (float4)
    };
    UINT numElements = ARRAYSIZE(layout);

    // Input Layout oluşturma
    hr = m_d3dDevice->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), m_inputLayout.GetAddressOf());
    if (FAILED(hr)) {
        fprintf(stderr, "Hata: Input Layout olusturulurken hata! HRESULT: %08X\n", hr);
        return false;
    }

    // Pixel Shader derleme
    vsBlob.Reset(); // Vertex shader blob'unu temizle, pixel shader için kullanacağız
    errorBlob.Reset(); // Hata blob'unu temizle

    // D3DCompile çağrısı düzeltildi: 10 parametre bekleniyor.
    hr = D3DCompile(g_PixelShader,            // pSrcData
        strlen(g_PixelShader),    // SrcDataSize
        nullptr,                  // pSourceName (opsiyonel)
        nullptr,                  // pDefines (opsiyonel)
        nullptr,                  // pInclude (opsiyonel)
        "PSMain",                 // pEntrypoint
        "ps_5_0",                 // pTarget
        D3DCOMPILE_ENABLE_STRICTNESS, // Flags1
        0,                        // Flags2
        vsBlob.GetAddressOf(),    // ppCode
        errorBlob.GetAddressOf()  // ppErrorMsgs (opsiyonel)
    );

    if (FAILED(hr))
    {
        if (errorBlob) fprintf(stderr, "Pixel Shader Derleme Hatasi: %s\n", (char*)errorBlob->GetBufferPointer());
        return false;
    }
    // CreatePixelShader çağrısı düzeltildi: 3 parametre (blob pointer, blob size, class linkages)
    hr = m_d3dDevice->CreatePixelShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf());
    if (FAILED(hr)) {
        fprintf(stderr, "Hata: Pixel Shader olusturulurken hata! HRESULT: %08X\n", hr);
        return false;
    }

    return true;
}

// Sabit tampon (constant buffer) oluşturma
bool D3D11Renderer::CreateConstantBuffer()
{
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBufferData); // Sabit tampon yapımızın boyutu
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    HRESULT hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_constantBuffer.GetAddressOf());
    if (FAILED(hr)) {
        fprintf(stderr, "Hata: Constant Buffer olusturulurken hata! HRESULT: %08X\n", hr);
        return false;
    }
    return true;
}

// Kamerayı ayarlama
void D3D11Renderer::SetupCamera()
{
    // Yüklenen mesh'in merkezini ve pozisyonunu kullanarak kamera matrisini oluştur
    // m_cameraPos ve m_cameraTarget, LoadMesh fonksiyonunda güncellenir.

    // Görünüm matrisini oluştur
    XMVECTOR Eye = XMVectorSet(m_cameraPos.x, m_cameraPos.y, m_cameraPos.z, 0.0f);
    XMVECTOR At = XMVectorSet(m_cameraTarget.x, m_cameraTarget.y, m_cameraTarget.z, 0.0f);
    XMVECTOR Up = XMVectorSet(m_cameraUp.x, m_cameraUp.y, m_cameraUp.z, 0.0f);
    m_viewMatrix = XMMatrixLookAtLH(Eye, At, Up); // Sol-el koordinat sistemi
}

// Dünya, Görünüm ve Projeksiyon matrislerini güncelleme
void D3D11Renderer::UpdateMatrices()
{
    // Dünya matrisi (şu an TranslateMesh tarafından güncelleniyor, burada dokunmuyoruz)

    // Görünüm matrisi
    SetupCamera(); // Kameranın pozisyonu değiştiğinde veya başlangıçta

    // Projeksiyon matrisi
    if (m_windowWidth > 0 && m_windowHeight > 0) {
        m_projectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV4, (FLOAT)m_windowWidth / (FLOAT)m_windowHeight, 0.01f, 1000.0f);
    }
}