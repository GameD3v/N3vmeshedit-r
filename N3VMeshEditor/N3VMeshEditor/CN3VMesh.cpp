#include "CN3VMesh.h" // VertexPositionColor tanımını buradan alacak
#include <fstream>      // Dosya işlemleri için
#include <iostream>     // Hata ayıklama için
#include <QDebug>       // qDebug için
#include <QString>      // QString için (qDebug ile kullanmak için)
#include <algorithm>    // std::min ve std::max için
#include <cmath>        // sqrt için
#include <cfloat>       // FLT_MAX, FLT_MIN için

CN3VMesh::CN3VMesh()
    : m_center({ 0.0f, 0.0f, 0.0f })
    , m_radius(0.0f)
{
}

CN3VMesh::~CN3VMesh()
{
    Clear();
}

void CN3VMesh::Clear()
{
    m_vertices.clear();
    m_indices.clear();
    m_center = { 0.0f, 0.0f, 0.0f };
    m_radius = 0.0f;
}

// Mesh dosyasından yükleme metodu
bool CN3VMesh::Load(const std::string& filename)
{
    Clear(); // Önceki verileri temizle

    std::ifstream file(filename, std::ios::binary); // Dosyayı ikili modda aç
    if (!file.is_open())
    {
        qDebug() << "Hata: Mesh dosyasi acilamadi: " << QString::fromStdString(filename);
        return false;
    }

    // Vertex sayısını oku
    unsigned int vertexCount;
    file.read(reinterpret_cast<char*>(&vertexCount), sizeof(unsigned int));
    m_vertices.resize(vertexCount); // Vertex vektörünü boyutlandır

    // Vertex verilerini oku
    file.read(reinterpret_cast<char*>(m_vertices.data()), vertexCount * sizeof(VertexPositionColor));

    // Index sayısını oku
    unsigned int indexCount;
    file.read(reinterpret_cast<char*>(&indexCount), sizeof(unsigned int));
    m_indices.resize(indexCount); // Index vektörünü boyutlandır

    // Index verilerini oku
    file.read(reinterpret_cast<char*>(m_indices.data()), indexCount * sizeof(unsigned short));

    file.close(); // Dosyayı kapat

    CalculateBoundingBox(); // Yüklendikten sonra bounding box'ı hesapla

    qDebug() << "Mesh dosyasi basariyla yuklendi: " << QString::fromStdString(filename)
        << " Vertices: " << vertexCount << " Indices: " << indexCount;
    return true;
}

void CN3VMesh::CalculateBoundingBox()
{
    if (m_vertices.empty())
    {
        m_center = { 0.0f, 0.0f, 0.0f };
        m_radius = 0.0f;
        return;
    }

    // Min ve Max koordinatları bul
    Vector3 minPos = { FLT_MAX, FLT_MAX, FLT_MAX };
    Vector3 maxPos = { FLT_MIN, FLT_MIN, FLT_MIN };

    for (const auto& v : m_vertices)
    {
        minPos.x = std::min(minPos.x, v.Position.x);
        minPos.y = std::min(minPos.y, v.Position.y);
        minPos.z = std::min(minPos.z, v.Position.z);

        maxPos.x = std::max(maxPos.x, v.Position.x);
        maxPos.y = std::max(maxPos.y, v.Position.y);
        maxPos.z = std::max(maxPos.z, v.Position.z);
    }

    // Merkezi hesapla
    m_center.x = (minPos.x + maxPos.x) / 2.0f;
    m_center.y = (minPos.y + maxPos.y) / 2.0f;
    m_center.z = (minPos.z + maxPos.z) / 2.0f;

    // Yarıçapı hesapla (merkezden en uzak vertex'e olan mesafe)
    float maxDistSq = 0.0f;
    for (const auto& v : m_vertices)
    {
        float dx = v.Position.x - m_center.x;
        float dy = v.Position.y - m_center.y;
        float dz = v.Position.z - m_center.z;
        maxDistSq = std::max(maxDistSq, dx * dx + dy * dy + dz * dz);
    }
    m_radius = std::sqrt(maxDistSq);
}