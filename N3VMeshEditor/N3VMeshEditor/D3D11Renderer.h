#pragma once

#include <d3d11.h>
#include <wrl/client.h> // Microsoft::WRL::ComPtr için
#include <DirectXMath.h> // DirectXMath tipleri için
#include "CN3VMesh.h" // CN3VMesh sınıfını dahil edin (VertexPositionColor tanımını buradan alacak)

// VertexPositionColor artık CN3VMesh.h'den geliyor, burada tekrar tanımlamaya gerek yok.

class D3D11Renderer
{
public:
    D3D11Renderer();
    ~D3D11Renderer();

    // Direct3D'yi başlatma
    bool Initialize(HWND hWnd, int width, int height);
    // Direct3D kaynaklarını temizleme
    void Shutdown();
    // Sahneyi çizme
    void Render();
    // Pencere boyutu değiştiğinde çağrılır
    void Resize(int width, int height);

    // Mesh verilerini Direct3D buffer'larına yükle
    void LoadMesh(const CN3VMesh& mesh);

    // Mesh'i belirli bir öteleme miktarıyla hareket ettirir
    void TranslateMesh(float x, float y, float z);

    // Renderer'ın kendisini döndürür (QDirect3D11Widget içinde kullanılabilir)
    ID3D11Device* GetDevice() const { return m_d3dDevice.Get(); } // Getter eklendi
    ID3D11DeviceContext* GetDeviceContext() const { return m_d3dContext.Get(); } // Getter eklendi

private:
    // Direct3D cihaz ve bağlam nesneleri
    Microsoft::WRL::ComPtr<ID3D11Device>           m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>    m_d3dContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain>         m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;

    // Shader ve input layout nesneleri
    Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>      m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_inputLayout;

    // Buffer nesneleri
    Microsoft::WRL::ComPtr<ID3D11Buffer>           m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>           m_indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>           m_constantBuffer; // Dünya, Görünüm, Projeksiyon matrisleri için

    // Pencere boyutları
    int m_windowWidth;
    int m_windowHeight;

    // Mesh vertex ve index sayıları
    size_t m_vertexCount;
    size_t m_indexCount;

    // Kamera ve dünya matrisleri
    DirectX::XMMATRIX m_worldMatrix;
    DirectX::XMMATRIX m_viewMatrix;
    DirectX::XMMATRIX m_projectionMatrix;

    // Kamera pozisyonu, hedefi ve yukarı vektörü
    DirectX::XMFLOAT3 m_cameraPos;
    DirectX::XMFLOAT3 m_cameraTarget;
    DirectX::XMFLOAT3 m_cameraUp;

    // Yardımcı fonksiyonlar
    bool CreateDeviceAndSwapChain(HWND hWnd, int width, int height);
    bool CreateShadersAndInputLayout();
    bool CreateConstantBuffer();
    void SetupCamera();
    void UpdateMatrices();
};