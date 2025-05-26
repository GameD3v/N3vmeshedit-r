//#pragma once
//
//#include <vector>
//#include <string> // std::string için dahil edildi
//#include <DirectXMath.h> // DirectXMath tipleri için (XMFLOAT3, XMFLOAT4)
//
//// Vertex yapısı: Pozisyon ve Renk içerir
//struct VertexPositionColor
//{
//    DirectX::XMFLOAT3 Position; // X, Y, Z koordinatları
//    DirectX::XMFLOAT4 Color;    // R, G, B, A renk değerleri
//};
//
//// Basit bir mesh sınıfı
//class CN3VMesh
//{
//public:
//    CN3VMesh();
//    ~CN3VMesh();
//
//    // Mesh dosyasından yükleme metodu
//    // Şimdi std::string alacak şekilde güncellendi
//    bool Load(const std::string& filename);
//    void Clear(); // Mesh verilerini temizle
//
//    const std::vector<VertexPositionColor>& GetVertices() const { return m_vertices; }
//    const std::vector<unsigned short>& GetIndices() const { return m_indices; }
//
//private:
//    std::vector<VertexPositionColor> m_vertices; // Vertex verileri
//    std::vector<unsigned short> m_indices;     // Index verileri (unsigned short, çünkü D3D11 için çoğu durumda yeterlidir)
//};