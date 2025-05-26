//#include "N3VMesh.h"
//#include <fstream>      // Dosya işlemleri için
//#include <iostream>     // Hata ayıklama için (isteğe bağlı, qDebug tercih edilebilir)
//#include <QDebug>       // qDebug için
//#include <QString>      // QString için (qDebug ile kullanmak için)
//
//CN3VMesh::CN3VMesh()
//{
//}
//
//CN3VMesh::~CN3VMesh()
//{
//    Clear();
//}
//
//void CN3VMesh::Clear()
//{
//    m_vertices.clear();
//    m_indices.clear();
//}
//
//// Mesh dosyasından yükleme metodu
//bool CN3VMesh::Load(const std::string& filename) // std::string alacak şekilde güncellendi
//{
//    Clear(); // Önceki verileri temizle
//
//    std::ifstream file(filename, std::ios::binary); // Dosyayı ikili modda aç
//    if (!file.is_open())
//    {
//        qDebug() << "Hata: Mesh dosyasi acilamadi: " << QString::fromStdString(filename);
//        return false;
//    }
//
//    // Vertex sayısını oku
//    unsigned int vertexCount;
//    file.read(reinterpret_cast<char*>(&vertexCount), sizeof(unsigned int));
//    m_vertices.resize(vertexCount); // Vertex vektörünü boyutlandır
//
//    // Vertex verilerini oku
//    file.read(reinterpret_cast<char*>(m_vertices.data()), vertexCount * sizeof(VertexPositionColor));
//
//    // Index sayısını oku
//    unsigned int indexCount;
//    file.read(reinterpret_cast<char*>(&indexCount), sizeof(unsigned int));
//    m_indices.resize(indexCount); // Index vektörünü boyutlandır
//
//    // Index verilerini oku
//    file.read(reinterpret_cast<char*>(m_indices.data()), indexCount * sizeof(unsigned short));
//
//    file.close(); // Dosyayı kapat
//
//    qDebug() << "Mesh dosyasi basariyla yuklendi: " << QString::fromStdString(filename)
//        << " Vertices: " << vertexCount << " Indices: " << indexCount;
//    return true;
//}