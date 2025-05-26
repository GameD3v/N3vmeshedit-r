#include "QDirect3D11Widget.h"
#include <QResizeEvent>
#include <QDebug> // qDebug için

QDirect3D11Widget::QDirect3D11Widget(QWidget* parent)
    : QWidget(parent)
    , m_initialized(false)
{
    // Pencerenin Direct3D ile çizilmesi için uygun bayrakları ayarla
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_NoSystemBackground); // Sistem arka planını çizmeyi devre dışı bırak

    // Odaklanılabilir yap ki klavye olaylarını alabilirsin (isteğe bağlı)
    setFocusPolicy(Qt::StrongFocus);
}

QDirect3D11Widget::~QDirect3D11Widget()
{
    // Renderer zaten QWidget'ın bir üyesi olduğu için otomatik olarak yok edilecektir.
    // Ancak açıkça shutdown çağrısı yapmak daha güvenli olabilir.
    if (m_initialized) {
        m_renderer.Shutdown();
    }
}

QPaintEngine* QDirect3D11Widget::paintEngine() const
{
    return nullptr; // Direct3D kullanıldığı için Qt'nin kendi PaintEngine'ini kullanma
}

void QDirect3D11Widget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    if (!m_initialized) {
        // İlk çizim olayında veya gösterildiğinde render'ı başlat
        if (m_renderer.Initialize((HWND)winId(), width(), height())) {
            m_initialized = true;
            qDebug() << "D3D11Renderer basariyla baslatildi.";
        }
        else {
            qDebug() << "Hata: D3D11Renderer baslatilamadi.";
            // Hata durumunda ek widget'ı devre dışı bırak veya mesaj göster
            return;
        }
    }
    // Renderer'ı çiz
    m_renderer.Render();
}

void QDirect3D11Widget::resizeEvent(QResizeEvent* event)
{
    if (m_initialized) {
        // Pencere boyutu değiştiğinde Direct3D'yi yeniden boyutlandır
        m_renderer.Resize(event->size().width(), event->size().height());
    }
    QWidget::resizeEvent(event); // Temel sınıfın resize olayını çağır
}

void QDirect3D11Widget::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);
    if (!m_initialized) {
        // Widget ilk gösterildiğinde Direct3D'yi başlat
        // paintEvent içinde de bu kontrol var, burası bir ek güvenlik katmanı.
        // Ancak genellikle render initialize edildiği zaman ShowEvent sonrası paintEvent çağrılır.
    }
    QWidget::showEvent(event);
}

void QDirect3D11Widget::hideEvent(QHideEvent* event)
{
    Q_UNUSED(event);
    // Widget gizlendiğinde Direct3D kaynaklarını serbest bırakmak istenebilir,
    // ancak genellikle uygulama kapanana kadar tutulur.
    QWidget::hideEvent(event);
}

void QDirect3D11Widget::LoadMesh(const CN3VMesh& mesh)
{
    // D3D11Renderer'daki LoadMesh metodunu çağırıyoruz
    m_renderer.LoadMesh(mesh);
}