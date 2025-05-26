#ifndef N3VMESHEDITOR_H
#define N3VMESHEDITOR_H

#include <QMainWindow>
#include "QDirect3D11Widget.h" // D3D11 widget'ınızı dahil edin
#include "CN3VMesh.h"          // CN3VMesh sınıfını dahil edin (VertexPositionColor'ı da buradan alacak)

// N3VMeshEditor.ui dosyasından oluşturulan UI sınıfının ileri bildirimi
// ui_N3VMeshEditor.h sadece .cpp dosyasında dahil edilmelidir.
namespace Ui {
    class N3VMeshEditor; // İleri bildirim
}

class N3VMeshEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit N3VMeshEditor(QWidget* parent = nullptr);
    ~N3VMeshEditor();

private slots:
    void onLoadMeshButtonClicked();
    void onTranslateXButtonClicked();
    void onTranslateYButtonClicked();
    void onRotateYawButtonClicked(); // Eğer dönüş için bir slot ekleyecekseniz

private:
    Ui::N3VMeshEditor* ui; // ui elemanlarına erişmek için (ileri bildirim sayesinde burada sorun olmaz)
    QDirect3D11Widget* m_d3dWidget; // Direct3D render widget'ımız
};

#endif // N3VMESHEDITOR_H