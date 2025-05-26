/********************************************************************************
** Form generated from reading UI file 'N3VMeshEditor.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_N3VMESHEDITOR_H
#define UI_N3VMESHEDITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_N3VMeshEditorClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *N3VMeshEditorClass)
    {
        if (N3VMeshEditorClass->objectName().isEmpty())
            N3VMeshEditorClass->setObjectName("N3VMeshEditorClass");
        N3VMeshEditorClass->resize(600, 400);
        menuBar = new QMenuBar(N3VMeshEditorClass);
        menuBar->setObjectName("menuBar");
        N3VMeshEditorClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(N3VMeshEditorClass);
        mainToolBar->setObjectName("mainToolBar");
        N3VMeshEditorClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(N3VMeshEditorClass);
        centralWidget->setObjectName("centralWidget");
        N3VMeshEditorClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(N3VMeshEditorClass);
        statusBar->setObjectName("statusBar");
        N3VMeshEditorClass->setStatusBar(statusBar);

        retranslateUi(N3VMeshEditorClass);

        QMetaObject::connectSlotsByName(N3VMeshEditorClass);
    } // setupUi

    void retranslateUi(QMainWindow *N3VMeshEditorClass)
    {
        N3VMeshEditorClass->setWindowTitle(QCoreApplication::translate("N3VMeshEditorClass", "N3VMeshEditor", nullptr));
    } // retranslateUi

};

namespace Ui {
    class N3VMeshEditorClass: public Ui_N3VMeshEditorClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_N3VMESHEDITOR_H
