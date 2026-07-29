#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QProgressBar>
#include <QFrame>
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
	bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
	void onSelectFile();
	void onClearFile();
	void onSelectGamesDir();
	void onSelectAddonsDir();
	void onStartExtraction();
	void onCancelExtraction();
	void onSaveSettings();
	void onExtractionLog(const QString &message);
	void onExtractionProgress(int current, int total);
	void onExtractionFinished(int returnCode);

private:
	void setupUI();
	void loadSettings();
	void setExtractionActive(bool active);
	void setFileSelected(const QString &path);
	void setFileCleared();

	// Icon factory
	QIcon makeIcon(const QString &type, const QColor &color, const QColor &disabledColor = QColor());

	// Drop zone / file display
	QWidget *m_dropZone;
	QLabel *m_dropIcon;
	QLabel *m_dropText;
	QLabel *m_dropSubtext;

	// File selected display
	QWidget *m_fileDisplay;
	QLineEdit *m_fileInput;
	QPushButton *m_clearFileBtn;

	QLineEdit *m_gamesDirInput;
	QLineEdit *m_addonsDirInput;
	QPushButton *m_selectGamesDirBtn;
	QPushButton *m_selectAddonsDirBtn;
	QPushButton *m_extractBtn;
	QPushButton *m_cancelBtn;
	QPushButton *m_saveBtn;
	QTextEdit *m_logArea;
	QLabel *m_statusLabel;
	QProgressBar *m_progressBar;

	ExtractWorker *m_worker;
	SettingsManager *m_settings;
};
