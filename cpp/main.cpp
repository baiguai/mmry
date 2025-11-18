#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <QShortcut>
#include <QKeySequence>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <iostream>

class ConsoleWindow : public QMainWindow {
    Q_OBJECT

public:
    ConsoleWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setupUI();
        setupHotkeys();
        setupSystemTray();
        
        // Initially hide the window
        hide();
    }

private slots:
    void showWindow() {
        show();
        raise();
        activateWindow();
        std::cout << "Window shown" << std::endl;
    }
    
    void hideWindow() {
        hide();
        std::cout << "Window hidden" << std::endl;
    }
    
    void toggleWindow() {
        if (isVisible()) {
            hideWindow();
        } else {
            showWindow();
        }
    }

private:
    void setupUI() {
        setWindowTitle("MMRY - Clipboard Manager");
        setFixedSize(800, 600);
        
        // Create central widget with console-style text edit
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);
        
        QTextEdit *console = new QTextEdit(this);
        console->setStyleSheet(
            "QTextEdit {"
            "background-color: #1e1e1e;"
            "color: #ffffff;"
            "font-family: 'JetBrains Mono', monospace;"
            "font-size: 14px;"
            "border: 1px solid #333;"
            "padding: 10px;"
            "}"
        );
        console->setText("MMRY Clipboard Manager\nPress Ctrl+Alt+C to toggle window\nPress Escape to hide\n\n> ");
        console->setReadOnly(true);
        
        layout->addWidget(console);
        setCentralWidget(centralWidget);
    }
    
    void setupHotkeys() {
        // Ctrl+Alt+C to show window
        QShortcut *showShortcut = new QShortcut(QKeySequence("Ctrl+Alt+C"), this);
        connect(showShortcut, &QShortcut::activated, this, &ConsoleWindow::showWindow);
        
        // Escape to hide window
        QShortcut *hideShortcut = new QShortcut(QKeySequence("Escape"), this);
        connect(hideShortcut, &QShortcut::activated, this, &ConsoleWindow::hideWindow);
    }
    
    void setupSystemTray() {
        if (QSystemTrayIcon::isSystemTrayAvailable()) {
            QSystemTrayIcon *trayIcon = new QSystemTrayIcon(this);
            trayIcon->setIcon(QIcon()); // You can set a proper icon later
            
            QMenu *trayMenu = new QMenu(this);
            QAction *showAction = trayMenu->addAction("Show/Hide");
            QAction *quitAction = trayMenu->addAction("Quit");
            
            connect(showAction, &QAction::triggered, this, &ConsoleWindow::toggleWindow);
            connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
            
            trayIcon->setContextMenu(trayMenu);
            trayIcon->show();
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    ConsoleWindow window;
    
    return app.exec();
}

#include "main.moc"