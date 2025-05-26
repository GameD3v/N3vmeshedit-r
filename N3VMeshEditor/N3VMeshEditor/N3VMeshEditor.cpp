#include "N3VMeshEditor.h"
#include "ui_N3VMeshEditor.h" // Eksik olan include burada eklendi

#include <QFileDialog>        // Dosya açma diyaloğu için
#include <QDebug>             // Debug mesajları için
#include <QVBoxLayout>        // Dikey düzen için
#include <QHBoxLayout>        // Yatay düzen için
#include <QPushButton>        // Butonlar için

N3VMeshEditor::N3VMeshEditor(QWidget* parent)
    : QMainWindow(parent),
    ui(new Ui::N3VMeshEditor) // ui nesnesini oluştur
{
    ui->setupUi(this); // UI'ı bu pencereye kur

    // Ana pencerenin merkezi bir widget'ı olmalı
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget); // Ana dikey layout

    // QDirect3D11Widget'ı oluştur ve ana layout'a ekle
    m_d3dWidget = new QDirect3D11Widget(this);
    mainLayout->addWidget(m_d3dWidget, 1); // Expand etmesi için stretch faktörü 1 verilebilir

    // Butonlar için yatay layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    // Butonları oluştur
    QPushButton* loadMeshButton = new QPushButton("Mesh Yükle", this);
    QPushButton* translateUpButton = new QPushButton("Küpü Yükselt", this);
    QPushButton* translateDownButton = new QPushButton("Küpü Alçalt", this);
    QPushButton* translateLeftButton = new QPushButton("Küpü Sola", this);
    QPushButton* translateRightButton = new QPushButton("Küpü Sağa", this);
    QPushButton* exitButton = new QPushButton("Çıkış", this);

    // Butonları yatay layout'a ekle
    buttonLayout->addWidget(loadMeshButton);
    buttonLayout->addWidget(translateUpButton);
    buttonLayout->addWidget(translateDownButton);
    buttonLayout->addWidget(translateLeftButton);
    buttonLayout->addWidget(translateRightButton);
    buttonLayout->addStretch(); // Boşluk ekle
    buttonLayout->addWidget(exitButton);

    // Buton layout'unu ana layout'a ekle
    mainLayout->addLayout(buttonLayout);

    // Buton sinyallerini slotlara bağla
    connect(loadMeshButton, &QPushButton::clicked, this, &N3VMeshEditor::onLoadMeshButtonClicked);
    connect(translateUpButton, &QPushButton::clicked, this, &N3VMeshEditor::onTranslateYButtonClicked);
    connect(translateDownButton, &QPushButton::clicked, this, &N3VMeshEditor::onTranslateYButtonClicked);
    connect(translateLeftButton, &QPushButton::clicked, this, &N3VMeshEditor::onTranslateXButtonClicked);
    connect(translateRightButton, &QPushButton::clicked, this, &N3VMeshEditor::onTranslateXButtonClicked);
    connect(exitButton, &QPushButton::clicked, this, &QMainWindow::close);

    // Pencere başlığını ve boyutunu ayarla
    setWindowTitle("N3VMeshEditor - DirectX 11 & Qt");
    resize(800, 600); // Başlangıç pencere boyutu
}

N3VMeshEditor::~N3VMeshEditor()
{
    delete ui; // UI objesini temizle
    // m_d3dWidget QMainWindow'ın bir çocuğu olduğu için Qt tarafından otomatik olarak silinecektir.
}

void N3VMeshEditor::onLoadMeshButtonClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Mesh Dosyası Seç", "", "N3VMesh Dosyaları (*.n3vmesh);;Tüm Dosyalar (*.*)");

    if (!filePath.isEmpty()) {
        CN3VMesh mesh;
        if (mesh.Load(filePath.toStdString())) {
            m_d3dWidget->LoadMesh(mesh);
            qDebug() << "Mesh başarıyla yüklendi: " << filePath;
        }
        else {
            qDebug() << "Hata: Mesh yüklenemedi: " << filePath;
        }
    }
}

void N3VMeshEditor::onTranslateXButtonClicked()
{
    QPushButton* senderButton = qobject_cast<QPushButton*>(sender());
    if (senderButton) {
        if (senderButton->text() == "Küpü Sağa") {
            m_d3dWidget->GetRenderer()->TranslateMesh(0.5f, 0.0f, 0.0f); // X ekseninde +0.5
        }
        else if (senderButton->text() == "Küpü Sola") {
            m_d3dWidget->GetRenderer()->TranslateMesh(-0.5f, 0.0f, 0.0f); // X ekseninde -0.5
        }
    }
}

void N3VMeshEditor::onTranslateYButtonClicked()
{
    QPushButton* senderButton = qobject_cast<QPushButton*>(sender());
    if (senderButton) {
        if (senderButton->text() == "Küpü Yükselt") {
            m_d3dWidget->GetRenderer()->TranslateMesh(0.0f, 0.5f, 0.0f); // Y ekseninde +0.5
        }
        else if (senderButton->text() == "Küpü Alçalt") {
            m_d3dWidget->GetRenderer()->TranslateMesh(0.0f, -0.5f, 0.0f); // Y ekseninde -0.5
        }
    }
}

void N3VMeshEditor::onRotateYawButtonClicked()
{
    // Bu metot şu an kullanılmıyor, ancak ekleme yaparsanız burada kullanabilirsiniz.
}
