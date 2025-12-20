#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include "UI/VTKWidget.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== DEBUG ASYNC MAIN: Starting ===";
    
    QMainWindow window;
    window.setWindowTitle("Debug Async STEP Loading");
    window.resize(800, 600);
    
    QWidget* centralWidget = new QWidget(&window);
    window.setCentralWidget(centralWidget);
    
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    
    QLabel* statusLabel = new QLabel("Ready to test async loading", centralWidget);
    layout->addWidget(statusLabel);
    
    UI::VTKWidget* vtkWidget = new UI::VTKWidget(centralWidget);
    layout->addWidget(vtkWidget);
    
    QPushButton* loadBtn = new QPushButton("Load MPX3500.STEP", centralWidget);
    layout->addWidget(loadBtn);
    
    QObject::connect(loadBtn, &QPushButton::clicked, [vtkWidget, statusLabel]() {
        qDebug() << "=== DEBUG: Load button clicked ===";
        statusLabel->setText("Loading STEP file...");
        
        QString filePath = "K:/vsCodeProjects/qtSpraySystem/data/model/MPX3500.STEP";
        qDebug() << "=== DEBUG: Calling LoadSTEPModel with:" << filePath;
        
        bool result = vtkWidget->LoadSTEPModel(filePath);
        qDebug() << "=== DEBUG: LoadSTEPModel returned:" << result;
    });
    
    window.show();
    
    qDebug() << "=== DEBUG ASYNC MAIN: Window shown, entering event loop ===";
    
    return app.exec();
}