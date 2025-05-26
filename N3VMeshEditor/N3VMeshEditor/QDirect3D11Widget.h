#ifndef QDIRECT3D11WIDGET_H
#define QDIRECT3D11WIDGET_H

#include <QWidget>
#include "D3D11Renderer.h" // D3D11Renderer sınıfını dahil edin

class QDirect3D11Widget : public QWidget
{
    Q_OBJECT

public:
    explicit QDirect3D11Widget(QWidget* parent = nullptr);
    ~QDirect3D11Widget();

    // D3D11Renderer objesine erişim için
    D3D11Renderer* GetRenderer() { return &m_renderer; }

    // Mesh yükleme fonksiyonunu public yapıyoruz ki N3VMeshEditor çağırabilsin.
    void LoadMesh(const CN3VMesh& mesh);

protected:
    virtual QPaintEngine* paintEngine() const override;
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void showEvent(QShowEvent* event) override;
    virtual void hideEvent(QHideEvent* event) override;

private:
    D3D11Renderer m_renderer; // DirectX 11 render'ımızı yöneten sınıf
    bool m_initialized; // Renderer'ın başlatılıp başlatılmadığını kontrol etmek için
};

#endif // QDIRECT3D11WIDGET_H