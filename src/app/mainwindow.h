/**
 * @file mainwindow.h
 * @brief Main window for the cppsnippets application
 */

#pragma once

#include <QMainWindow>
#include <memory>

class QListWidget;
class QTextEdit;
class QLineEdit;

namespace cppsnippets {
class SnippetManager;
}

/**
 * @brief Main application window
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~MainWindow() override;

private slots:
    void onNewSnippet();
    void onDeleteSnippet();
    void onSaveSnippet();
    void onSnippetSelected();
    void onSearch(const QString& text);

private:
    void setupUi();
    void setupMenus();
    void refreshSnippetList();
    
    std::unique_ptr<cppsnippets::SnippetManager> m_snippetManager;
    QListWidget* m_snippetList;
    QLineEdit* m_prefixEdit;
    QTextEdit* m_bodyEdit;
    QLineEdit* m_descriptionEdit;
    QLineEdit* m_searchEdit;
};
