#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QProgressBar>
#include <QFrame>
#include <QComboBox>
#include <QListWidget>
#include <QStringList>
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
	void onSelectFiles();
	void onClearFiles();
	void onSelectGamesDir();
	void onSelectAddonsDir();
	void onStartExtraction();
	void onCancelExtraction();
	void onSaveSettings();
	void onExtractionLog(const QString &message);
	void onExtractionProgress(int current, int total);
	void onExtractionFinished(int returnCode);
	void onLanguageChanged(int index);
	void onCopyLog();
	void onDetectShadPS4();

private:
	void setupUI();
	void loadSettings();
	void setExtractionActive(bool active);
	void addFilesToList(const QStringList &paths);
	void clearFileList();
	void updateFileListDisplay();
	void retranslateUI();

	// Icon factory
	QIcon makeIcon(const QString &type, const QColor &color, const QColor &disabledColor = QColor());

	// Get PKG type name for display
	QString getPkgTypeName(int typeCode) const;

	// Drop zone / file display
	QWidget *m_dropZone;
	QLabel *m_dropIcon;
	QLabel *m_dropText;
	QLabel *m_dropSubtext;

	// File list display (multi-file support)
	QWidget *m_fileListWidget;
	QListWidget *m_fileList;
	QLabel *m_fileCountLabel;
	QPushButton *m_clearFilesBtn;

	QLineEdit *m_gamesDirInput;
	QLineEdit *m_addonsDirInput;
	QPushButton *m_selectGamesDirBtn;
	QPushButton *m_selectAddonsDirBtn;
	QPushButton *m_extractBtn;
	QPushButton *m_cancelBtn;
	QPushButton *m_saveBtn;
	QPushButton *m_detectShadPS4Btn;
	QTextEdit *m_logArea;
	QLabel *m_statusLabel;
	QProgressBar *m_progressBar;
	QPushButton *m_copyLogBtn;

	// Labels needing retranslation
	QLabel *m_dirTitleLabel;
	QLabel *m_lblGames;
	QLabel *m_lblAddons;
	QLabel *m_logTitleLabel;
	QLabel *m_langLabel;

	// Selected files
	QStringList m_selectedFiles;

	ExtractWorker *m_worker;
	SettingsManager *m_settings;
	QComboBox *m_languageCombo;
};
