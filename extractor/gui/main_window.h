#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QProgressBar>
#include "extract_worker.h"
#include "settings_manager.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

protected:
	void dragEnterEvent(QDragEnterEvent *event) override;
	void dropEvent(QDropEvent *event) override;

private slots:
	void onSelectFile();
	void onSelectGamesDir();
	void onSelectAddonsDir();
	void onStartExtraction();
	void onSaveSettings();
	void onExtractionLog(const QString &message);
	void onExtractionFinished(int returnCode);

private:
	void setupUI();
	void loadSettings();

	QLineEdit *m_fileInput;
	QLineEdit *m_gamesDirInput;
	QLineEdit *m_addonsDirInput;
	QPushButton *m_extractBtn;
	QTextEdit *m_logArea;
	QLabel *m_statusLabel;
	QProgressBar *m_progressBar;

	ExtractWorker *m_worker;
	SettingsManager *m_settings;
};
