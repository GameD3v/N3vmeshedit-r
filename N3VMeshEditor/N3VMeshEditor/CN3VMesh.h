#pragma once

#include <vector>
#include <string> // std::string için
#include <DirectXMath.h> // DirectXMath tipleri için (XMFLOAT3, XMFLOAT4)

// Vertex yapısı: Pozisyon ve Renk içerir (SADECE BURADA TANIMLI)
struct VertexPositionColor
{
    DirectX::XMFLOAT3 Position; // X, Y, Z koordinatları
    DirectX::XMFLOAT4 Color;    // R, G, B, A renk değerleri
};

// Basit bir 3D vektör yapısı (CN3VMesh'in iç kullanımı için)
struct Vector3
{
    float x, y, z;
};

// Basit bir mesh sınıfı
class CN3VMesh
{
public:
    CN3VMesh();
    ~CN3VMesh();

    // Mesh dosyasından yükleme metodu
    bool Load(const std::string& filename);
    void Clear(); // Mesh verilerini temizle

    // Vertex ve Index verilerine erişim fonksiyonları
    const std::vector<VertexPositionColor>& GetVertices() const { return m_vertices; }
    const std::vector<unsigned short>& GetIndices() const { return m_indices; }

    // Vertex ve Index sayılarını döndüren yeni fonksiyonlar
    size_t GetVertexCount() const { return m_vertices.size(); }
    size_t GetIndexCount() const { return m_indices.size(); }

    // Mesh merkezini ve yarıçapını döndüren fonksiyonlar
    Vector3 GetCenter() const { return m_center; }
    float GetRadius() const { return m_radius; }

private:
    std::vector<VertexPositionColor> m_vertices; // Vertex verileri
    std::vector<unsigned short> m_indices;     // Index verileri (unsigned short, çünkü D3D11 için çoğu durumda yeterlidir)
    Vector3 m_center; // Mesh'in merkezi
    float m_radius;   // Mesh'i çevreleyen kürenin yarıçapı

    void CalculateBoundingBox(); // Mesh'in sınırlayıcı kutusunu hesaplar ve merkez/yarıçapı günceller
};