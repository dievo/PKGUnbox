#include "main_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QIcon>
#include <QApplication>
#include <QClipboard>
#include <QScrollBar>
#include <QSizePolicy>
#include <QPainter>
#include <QPainterPath>
#include <QTranslator>
#include <QDebug>
#include <QTimer>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_worker(nullptr)
	, m_settings(new SettingsManager(this))
{
	setupUI();
	loadSettings();
}

MainWindow::~MainWindow() {
	if (m_worker && m_worker->isRunning()) {
		m_worker->quit();
		m_worker->wait(3000);
		if (m_worker->isRunning()) {
			m_worker->terminate();
		}
	}
}

// ========================================
QIcon MainWindow::makeIcon(const QString &type, const QColor &color, const QColor &disabledColor) {
	const int S = 32;
	QColor dc = disabledColor.isValid() ? disabledColor : QColor("#4e5058");

	auto drawShape = [&](QPainter &p, const QColor &c) {
		p.setPen(QPen(c, 2.0));
		p.setBrush(Qt::NoBrush);

		if (type == "folder") {
			QPainterPath path;
			path.moveTo(4, 12);
			path.lineTo(4, 8);
			path.lineTo(10, 8);
			path.lineTo(12, 5);
			path.lineTo(22, 5);
			path.lineTo(22, 26);
			path.lineTo(4, 26);
			path.closeSubpath();
			p.drawPath(path);
		} else if (type == "box") {
			p.drawRect(6, 14, 14, 12);
			QPolygon top;
			top << QPoint(6, 14) << QPoint(10, 8) << QPoint(24, 8) << QPoint(20, 14);
			p.drawPolygon(top);
			QPolygon side;
			side << QPoint(20, 14) << QPoint(24, 8) << QPoint(24, 20) << QPoint(20, 26);
			p.drawPolygon(side);
		} else if (type == "file") {
			QPolygon file;
			file << QPoint(8, 4) << QPoint(20, 4) << QPoint(24, 8)
			     << QPoint(24, 28) << QPoint(8, 28);
			p.drawPolygon(file);
			p.drawLine(20, 4, 20, 8);
			p.drawLine(20, 8, 24, 8);
		} else if (type == "save") {
			p.drawRect(5, 4, 22, 24);
			p.drawRect(10, 4, 8, 8);
			p.fillRect(11, 5, 6, 6, c);
			p.drawRect(10, 18, 12, 8);
		} else if (type == "cancel") {
			p.drawEllipse(4, 4, 24, 24);
			p.drawLine(11, 11, 21, 21);
			p.drawLine(21, 11, 11, 21);
		} else if (type == "check") {
			p.drawEllipse(4, 4, 24, 24);
			QPainterPath check;
			check.moveTo(10, 16);
			check.lineTo(14, 22);
			check.lineTo(22, 10);
			p.drawPath(check);
		} else if (type == "info") {
			p.drawEllipse(4, 4, 24, 24);
			p.fillRect(14, 10, 3, 3, c);
			p.fillRect(14, 16, 3, 8, c);
		} else if (type == "download") {
			p.drawLine(16, 4, 16, 20);
			p.drawLine(10, 14, 16, 22);
			p.drawLine(22, 14, 16, 22);
			p.drawLine(6, 26, 26, 26);
		} else if (type == "settings") {
			// Gear icon — circle with teeth
			p.drawEllipse(8, 8, 16, 16);
			p.drawEllipse(11, 11, 10, 10);
			// Teeth (8 small lines around the circle)
			for (int i = 0; i < 8; i++) {
				double angle = i * M_PI / 4.0;
				int x1 = static_cast<int>(16 + 8 * cos(angle));
				int y1 = static_cast<int>(16 + 8 * sin(angle));
				int x2 = static_cast<int>(16 + 11 * cos(angle));
				int y2 = static_cast<int>(16 + 11 * sin(angle));
				p.drawLine(x1, y1, x2, y2);
			}
		} else if (type == "search") {
			// Magnifying glass — circle + handle
			p.drawEllipse(5, 4, 16, 16);
			p.setPen(QPen(c, 3.0));
			p.drawLine(18, 17, 26, 26);
		}
	};

	QIcon icon;

	// Normal state
	QPixmap pixNormal(S, S);
	pixNormal.fill(Qt::transparent);
	QPainter pn(&pixNormal);
	pn.setRenderHint(QPainter::Antialiasing);
	drawShape(pn, color);
	pn.end();
	icon.addPixmap(pixNormal, QIcon::Normal, QIcon::Off);

	// Disabled state
	QPixmap pixDisabled(S, S);
	pixDisabled.fill(Qt::transparent);
	QPainter pd(&pixDisabled);
	pd.setRenderHint(QPainter::Antialiasing);
	drawShape(pd, dc);
	pd.end();
	icon.addPixmap(pixDisabled, QIcon::Disabled, QIcon::Off);

	return icon;
}

void MainWindow::setupUI() {
	setWindowTitle(QString("PKGUnbox - v%1").arg(PKGUNBOX_VERSION));
	setMinimumSize(700, 580);
	setAcceptDrops(true);

	// Window icon
	QString appDir = QApplication::applicationDirPath();
	QString iconPath = appDir + "/../share/pkgunbox/icon.png";
	if (!QFileInfo::exists(iconPath)) {
		iconPath = appDir + "/icon.png";
	}
	if (QFileInfo::exists(iconPath)) {
		setWindowIcon(QIcon(iconPath));
	}

	// Color palette
	QColor colFile("#7aa2f7");       // Blue — file/drop zone
	QColor colGames("#22c55e");      // Green — games
	QColor colAddons("#bb9af7");     // Purple — addons/DLCs
	QColor colExtract("#5865f2");    // Blurple — extract (primary)
	QColor colSave("#22c55e");       // Green — save (positive)
	QColor colCancel("#f7768e");     // Red — cancel/danger
	QColor colDetect("#f0c674");     // Yellow — detect shadPS4

	auto *central = new QWidget(this);
	setCentralWidget(central);
	auto *mainLayout = new QVBoxLayout(central);
	mainLayout->setSpacing(6);
	mainLayout->setContentsMargins(10, 10, 10, 10);

	// ========================================
	// === Drop Zone (always visible, clickable)
	// ========================================
	m_dropZone = new QWidget();
	m_dropZone->setObjectName("dropZone");
	m_dropZone->setFixedHeight(168);
	m_dropZone->setCursor(Qt::PointingHandCursor);
	m_dropZone->installEventFilter(this);

	auto *dropLayout = new QVBoxLayout(m_dropZone);
	dropLayout->setAlignment(Qt::AlignCenter);
	dropLayout->setSpacing(2);

	m_dropIcon = new QLabel();
	m_dropIcon->setPixmap(makeIcon("file", colFile).pixmap(32, 32));
	m_dropIcon->setAlignment(Qt::AlignCenter);
	dropLayout->addWidget(m_dropIcon);

	m_dropText = new QLabel(tr("Drag .pkg files here"));
	m_dropText->setObjectName("dropText");
	m_dropText->setAlignment(Qt::AlignCenter);
	dropLayout->addWidget(m_dropText);

	m_dropSubtext = new QLabel(tr("or click to browse (multiple files supported)"));
	m_dropSubtext->setObjectName("dropSubtext");
	m_dropSubtext->setAlignment(Qt::AlignCenter);
	dropLayout->addWidget(m_dropSubtext);

	mainLayout->addWidget(m_dropZone);

	// ========================================
	// === File List (hidden by default, shows selected files)
	// ========================================
	m_fileListWidget = new QWidget();
	m_fileListWidget->setVisible(false);
	auto *fileListLayout = new QVBoxLayout(m_fileListWidget);
	fileListLayout->setContentsMargins(0, 0, 0, 0);
	fileListLayout->setSpacing(4);

	// Header row with count, add button, and clear button
	auto *fileListHeader = new QHBoxLayout();
	m_fileCountLabel = new QLabel(tr("Selected Files (0)"));
	m_fileCountLabel->setObjectName("fileCountLabel");
	fileListHeader->addWidget(m_fileCountLabel);
	fileListHeader->addStretch();

	m_addFilesBtn = new QPushButton(tr("+ Add"));
	m_addFilesBtn->setObjectName("addFilesBtn");
	m_addFilesBtn->setFixedHeight(24);
	m_addFilesBtn->setToolTip(tr("Add more PKG files"));
	connect(m_addFilesBtn, &QPushButton::clicked, this, &MainWindow::onSelectFiles);
	fileListHeader->addWidget(m_addFilesBtn);

	m_clearFilesBtn = new QPushButton(tr("Clear All"));
	m_clearFilesBtn->setObjectName("clearFilesBtn");
	m_clearFilesBtn->setFixedHeight(24);
	m_clearFilesBtn->setToolTip(tr("Clear all selected files"));
	connect(m_clearFilesBtn, &QPushButton::clicked, this, &MainWindow::onClearFiles);
	fileListHeader->addWidget(m_clearFilesBtn);

	fileListLayout->addLayout(fileListHeader);

	// File list widget
	m_fileList = new QListWidget();
	m_fileList->setObjectName("fileList");
	m_fileList->setFixedHeight(140);
	fileListLayout->addWidget(m_fileList);

	mainLayout->addWidget(m_fileListWidget);

	// ========================================
	// === Destination Directories + Save
	// ========================================
	auto *dirContainer = new QWidget();
	dirContainer->setObjectName("dirContainer");
	auto *dirOuter = new QVBoxLayout(dirContainer);
	dirOuter->setContentsMargins(0, 0, 0, 0);
	dirOuter->setSpacing(0);

	// Title bar with Save button
	auto *dirTitleBar = new QHBoxLayout();
	dirTitleBar->setContentsMargins(10, 6, 10, 2);
	auto *dirTitleLabel = new QLabel(tr("Destination"));
	dirTitleLabel->setObjectName("groupTitle");
	m_dirTitleLabel = dirTitleLabel;
	dirTitleBar->addWidget(dirTitleLabel);
	dirTitleBar->addStretch();

	m_detectShadPS4Btn = new QPushButton();
	m_detectShadPS4Btn->setIcon(makeIcon("search", colDetect));
	m_detectShadPS4Btn->setText(tr("Auto-detect shadPS4"));
	m_detectShadPS4Btn->setObjectName("detectBtn");
	m_detectShadPS4Btn->setFixedHeight(24);
	m_detectShadPS4Btn->setToolTip(tr("Click to auto-detect shadPS4 installation and configure directories"));
	connect(m_detectShadPS4Btn, &QPushButton::clicked, this, &MainWindow::onDetectShadPS4);
	dirTitleBar->addWidget(m_detectShadPS4Btn);

	// Spacer between Detect and Save buttons
	auto *btnSpacer = new QSpacerItem(8, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
	dirTitleBar->addSpacerItem(btnSpacer);

	m_saveBtn = new QPushButton();
	m_saveBtn->setIcon(makeIcon("settings", colSave));
	m_saveBtn->setText(tr("Save"));
	m_saveBtn->setObjectName("saveBtn");
	m_saveBtn->setFixedHeight(24);
	m_saveBtn->setToolTip(tr("Click to save destination settings"));
	connect(m_saveBtn, &QPushButton::clicked, this, &MainWindow::onSaveSettings);
	dirTitleBar->addWidget(m_saveBtn);

	dirOuter->addLayout(dirTitleBar);

	// Grid for directory inputs
	auto *dirGrid = new QGridLayout();
	dirGrid->setSpacing(8);
	dirGrid->setContentsMargins(10, 4, 10, 10);

	// Games directory
	auto *lblGames = new QLabel(tr("Games:"));
	lblGames->setFixedWidth(55);
	m_lblGames = lblGames;
	dirGrid->addWidget(lblGames, 0, 0);

	m_gamesDirInput = new QLineEdit();
	m_gamesDirInput->setMinimumHeight(30);
	dirGrid->addWidget(m_gamesDirInput, 0, 1);

	m_selectGamesDirBtn = new QPushButton();
	m_selectGamesDirBtn->setIcon(makeIcon("folder", colGames));
	m_selectGamesDirBtn->setObjectName("browseBtn");
	m_selectGamesDirBtn->setFixedSize(30, 30);
	m_selectGamesDirBtn->setToolTip(tr("Click to select games directory"));
	connect(m_selectGamesDirBtn, &QPushButton::clicked, this, &MainWindow::onSelectGamesDir);
	dirGrid->addWidget(m_selectGamesDirBtn, 0, 2);

	// Addons directory
	auto *lblAddons = new QLabel(tr("DLCs:"));
	lblAddons->setFixedWidth(55);
	m_lblAddons = lblAddons;
	dirGrid->addWidget(lblAddons, 1, 0);

	m_addonsDirInput = new QLineEdit();
	m_addonsDirInput->setMinimumHeight(30);
	dirGrid->addWidget(m_addonsDirInput, 1, 1);

	m_selectAddonsDirBtn = new QPushButton();
	m_selectAddonsDirBtn->setIcon(makeIcon("folder", colAddons));
	m_selectAddonsDirBtn->setObjectName("browseBtn");
	m_selectAddonsDirBtn->setFixedSize(30, 30);
	m_selectAddonsDirBtn->setToolTip(tr("Click to select DLCs directory"));
	connect(m_selectAddonsDirBtn, &QPushButton::clicked, this, &MainWindow::onSelectAddonsDir);
	dirGrid->addWidget(m_selectAddonsDirBtn, 1, 2);

	dirOuter->addLayout(dirGrid);
	mainLayout->addWidget(dirContainer);

	// ========================================
	// === Extract Button (centered, prominent)
	// ========================================
	auto *extractLayout = new QHBoxLayout();
	extractLayout->setAlignment(Qt::AlignCenter);

	m_extractBtn = new QPushButton();
	m_extractBtn->setIcon(makeIcon("box", QColor("#ffffff")));
	m_extractBtn->setText(tr("Extract"));
	m_extractBtn->setObjectName("extractBtn");
	m_extractBtn->setMinimumSize(160, 40);
	m_extractBtn->setEnabled(false);
	m_extractBtn->setToolTip(tr("Start PKG extraction"));
	connect(m_extractBtn, &QPushButton::clicked, this, &MainWindow::onStartExtraction);
	extractLayout->addWidget(m_extractBtn);

	m_cancelBtn = new QPushButton();
	m_cancelBtn->setIcon(makeIcon("cancel", QColor("#ffffff")));
	m_cancelBtn->setText(tr("Cancel"));
	m_cancelBtn->setObjectName("cancelBtn");
	m_cancelBtn->setMinimumSize(140, 40);
	m_cancelBtn->hide();
	connect(m_cancelBtn, &QPushButton::clicked, this, &MainWindow::onCancelExtraction);
	extractLayout->addWidget(m_cancelBtn);

	mainLayout->addLayout(extractLayout);

	// ========================================
	// === Progress Bar (always visible)
	// ========================================
	m_progressBar = new QProgressBar();
	m_progressBar->setRange(0, 100);
	m_progressBar->setValue(0);
	m_progressBar->setFormat("%p%");
	m_progressBar->setFixedHeight(28);
	m_progressBar->setAlignment(Qt::AlignCenter);
	mainLayout->addWidget(m_progressBar);

	// ========================================
	// === Separator
	// ========================================
	auto *separator = new QFrame();
	separator->setFrameShape(QFrame::HLine);
	separator->setFrameShadow(QFrame::Sunken);
	mainLayout->addWidget(separator);

	// ========================================
	// === Log (always visible, fills remaining space)
	// ========================================
	auto *logTitleBar = new QHBoxLayout();
	logTitleBar->setContentsMargins(0, 0, 0, 0);

	auto *logLabel = new QLabel(tr("Log"));
	logLabel->setObjectName("logLabel");
	m_logTitleLabel = logLabel;
	logTitleBar->addWidget(logLabel);
	logTitleBar->addStretch();

	m_clearLogBtn = new QPushButton(tr("Clear Log"));
	m_clearLogBtn->setObjectName("clearLogBtn");
	m_clearLogBtn->setToolTip(tr("Clear log contents"));
	m_clearLogBtn->setEnabled(false);
	connect(m_clearLogBtn, &QPushButton::clicked, this, &MainWindow::onClearLog);
	logTitleBar->addWidget(m_clearLogBtn);

	m_copyLogBtn = new QPushButton(tr("Copy Log"));
	m_copyLogBtn->setObjectName("copyLogBtn");
	m_copyLogBtn->setToolTip(tr("Copy log contents to clipboard"));
	m_copyLogBtn->setEnabled(false);
	connect(m_copyLogBtn, &QPushButton::clicked, this, &MainWindow::onCopyLog);
	logTitleBar->addWidget(m_copyLogBtn);

	mainLayout->addLayout(logTitleBar);

	m_logArea = new QTextEdit();
	m_logArea->setReadOnly(true);
	m_logArea->setPlaceholderText(tr("Waiting for operation..."));
	m_logArea->setMinimumHeight(80);
	connect(m_logArea, &QTextEdit::textChanged, this, &MainWindow::updateLogButtons);
	mainLayout->addWidget(m_logArea, 1);

	// ========================================
	// === Status + Language (same row)
	// ========================================
	auto *statusLangLayout = new QHBoxLayout();
	statusLangLayout->setContentsMargins(0, 0, 0, 0);

	m_statusLabel = new QLabel(tr("Ready"));
	m_statusLabel->setObjectName("statusLabel");
	statusLangLayout->addWidget(m_statusLabel);
	statusLangLayout->addStretch();

	auto *langLabel = new QLabel(tr("Language:"));
	langLabel->setObjectName("langLabel");
	m_langLabel = langLabel;
	statusLangLayout->addWidget(langLabel);

	m_languageCombo = new QComboBox();
	m_languageCombo->addItem("English", "en");
	m_languageCombo->addItem("Portugues (BR)", "pt_BR");
	m_languageCombo->addItem("Espanol", "es");
	m_languageCombo->setFixedWidth(140);
	connect(m_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &MainWindow::onLanguageChanged);
	statusLangLayout->addWidget(m_languageCombo);

	mainLayout->addLayout(statusLangLayout);
}

void MainWindow::loadSettings() {
	m_gamesDirInput->setText(m_settings->gamesDir());
	m_addonsDirInput->setText(m_settings->addonsDir());

	// Load saved language
	QString savedLang = m_settings->language();
	if (!savedLang.isEmpty()) {
		int idx = m_languageCombo->findData(savedLang);
		if (idx >= 0) {
			m_languageCombo->setCurrentIndex(idx);
		}
	}
}

void MainWindow::setExtractionActive(bool active) {
	m_extractBtn->setEnabled(!active);
	m_extractBtn->setVisible(!active);
	m_cancelBtn->setEnabled(active);
	m_cancelBtn->setVisible(active);
	m_selectGamesDirBtn->setEnabled(!active);
	m_selectAddonsDirBtn->setEnabled(!active);
	m_saveBtn->setEnabled(!active);
	m_clearFilesBtn->setEnabled(!active);
	m_addFilesBtn->setEnabled(!active);
	m_detectShadPS4Btn->setEnabled(!active);

	// Disable remove buttons in file list during extraction
	for (int i = 0; i < m_fileList->count(); ++i) {
		QListWidgetItem *item = m_fileList->item(i);
		if (QWidget *widget = m_fileList->itemWidget(item)) {
			if (QPushButton *removeBtn = widget->findChild<QPushButton*>("removeFileBtn")) {
				removeBtn->setEnabled(!active);
			}
		}
	}

	if (active) {
		m_cancelBtn->setText(tr("Cancel"));
		m_progressBar->setValue(0);
		m_progressBar->setProperty("complete", false);
		m_progressBar->style()->unpolish(m_progressBar);
		m_progressBar->style()->polish(m_progressBar);
		m_logArea->clear();
	}
}

// ========================================
// === File Selection (Multi-file support)
// ========================================

void MainWindow::onSelectFiles() {
	QStringList paths = QFileDialog::getOpenFileNames(
		this, tr("Select PKG files"), "",
		tr("PKG files (*.pkg);;All files (*)")
	);
	if (!paths.isEmpty()) {
		addFilesToList(paths);
	}
}

void MainWindow::onClearFiles() {
	clearFileList();
}

void MainWindow::addFilesToList(const QStringList &paths) {
	for (const QString &path : paths) {
		// Avoid duplicates
		if (!m_selectedFiles.contains(path)) {
			m_selectedFiles.append(path);
		}
	}
	updateFileListDisplay();
}

void MainWindow::clearFileList() {
	m_selectedFiles.clear();
	updateFileListDisplay();

	// Show drop zone, hide file list
	m_fileListWidget->setVisible(false);
	m_dropZone->setVisible(true);
	m_extractBtn->setEnabled(false);
	m_statusLabel->setText(tr("Ready"));
	m_progressBar->setValue(0);
	m_progressBar->setProperty("complete", false);
	m_progressBar->style()->unpolish(m_progressBar);
	m_progressBar->style()->polish(m_progressBar);
}

void MainWindow::updateFileListDisplay() {
	m_fileList->clear();

	if (m_selectedFiles.isEmpty()) {
		m_fileCountLabel->setText(tr("Selected Files (0)"));
		return;
	}

	m_fileCountLabel->setText(tr("Selected Files (%1)").arg(m_selectedFiles.size()));

	for (int i = 0; i < m_selectedFiles.size(); ++i) {
		const QString &path = m_selectedFiles[i];
		QFileInfo info(path);
		QString fileName = info.fileName();
		QString sizeStr;

		qint64 size = info.size();
		if (size >= 1073741824LL) {
			sizeStr = QString("%1 GB").arg(size / 1073741824.0, 0, 'f', 1);
		} else if (size >= 1048576LL) {
			sizeStr = QString("%1 MB").arg(size / 1048576.0, 0, 'f', 1);
		} else {
			sizeStr = QString("%1 KB").arg(size / 1024.0, 0, 'f', 1);
		}

		// Create custom widget for each file row
		auto *rowWidget = new QWidget();
		auto *rowLayout = new QHBoxLayout(rowWidget);
		rowLayout->setContentsMargins(4, 2, 4, 2);
		rowLayout->setSpacing(6);

		// File icon
		auto *iconLabel = new QLabel();
		QFileInfo fileInfo(path);
		if (fileInfo.suffix().toLower() == "pkg") {
			iconLabel->setPixmap(makeIcon("box", QColor("#7aa2f7")).pixmap(16, 16));
		} else {
			iconLabel->setPixmap(makeIcon("file", QColor("#cdd6f4")).pixmap(16, 16));
		}
		iconLabel->setFixedSize(16, 16);
		rowLayout->addWidget(iconLabel);

		// File name
		auto *nameLabel = new QLabel(fileName);
		nameLabel->setToolTip(path);
		nameLabel->setObjectName("fileItemName");
		rowLayout->addWidget(nameLabel, 1);

		// File size
		auto *sizeLabel = new QLabel(sizeStr);
		sizeLabel->setObjectName("fileItemSize");
		sizeLabel->setFixedWidth(60);
		rowLayout->addWidget(sizeLabel);

		// Remove button
		auto *removeBtn = new QPushButton();
		removeBtn->setIcon(makeIcon("cancel", QColor("#f87171")));
		removeBtn->setObjectName("removeFileBtn");
		removeBtn->setFixedSize(20, 20);
		removeBtn->setToolTip(tr("Remove file"));
		// Use lambda to capture index
		connect(removeBtn, &QPushButton::clicked, this, [this, i]() {
			if (i >= 0 && i < m_selectedFiles.size()) {
				m_selectedFiles.removeAt(i);
				updateFileListDisplay();
				if (m_selectedFiles.isEmpty()) {
					m_fileListWidget->setVisible(false);
					m_dropZone->setVisible(true);
					m_extractBtn->setEnabled(false);
					m_statusLabel->setText(tr("Ready"));
				} else {
					m_statusLabel->setText(tr("%1 file(s) selected").arg(m_selectedFiles.size()));
				}
			}
		});
		rowLayout->addWidget(removeBtn);

		// Add row to list
		auto *item = new QListWidgetItem();
		item->setSizeHint(QSize(0, 28));
		m_fileList->addItem(item);
		m_fileList->setItemWidget(item, rowWidget);
	}

	// Show file list, hide drop zone
	m_fileListWidget->setVisible(true);
	m_dropZone->setVisible(false);
	m_extractBtn->setEnabled(true);
	m_statusLabel->setText(tr("%1 file(s) selected").arg(m_selectedFiles.size()));
}

void MainWindow::onSelectGamesDir() {
	QString dir = QFileDialog::getExistingDirectory(
		this, tr("Select games directory"), m_gamesDirInput->text()
	);
	if (!dir.isEmpty()) {
		m_gamesDirInput->setText(dir);
	}
}

void MainWindow::onSelectAddonsDir() {
	QString dir = QFileDialog::getExistingDirectory(
		this, tr("Select DLCs/Updates directory"), m_addonsDirInput->text()
	);
	if (!dir.isEmpty()) {
		m_addonsDirInput->setText(dir);
	}
}

// ========================================
// === Extraction (Batch support)
// ========================================

void MainWindow::onStartExtraction() {
	if (m_selectedFiles.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("Please select PKG file(s)."));
		return;
	}

	QString gamesDir = m_gamesDirInput->text();
	if (gamesDir.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("Please configure the games directory."));
		return;
	}

	setExtractionActive(true);

	m_logArea->append("======================================");
	m_logArea->append(QString("Starting batch extraction: %1 file(s)").arg(m_selectedFiles.size()));
	m_logArea->append(QString("Games directory: %1").arg(gamesDir));
	if (!m_addonsDirInput->text().isEmpty()) {
		m_logArea->append(QString("DLCs directory: %1").arg(m_addonsDirInput->text()));
	}
	m_logArea->append("======================================");

	m_worker = new ExtractWorker(m_selectedFiles, gamesDir, m_addonsDirInput->text(), this);
	connect(m_worker, &ExtractWorker::log, this, &MainWindow::onExtractionLog);
	connect(m_worker, &ExtractWorker::overallProgress, this, &MainWindow::onOverallProgress);
	connect(m_worker, &ExtractWorker::batchProgress, this, &MainWindow::onBatchProgress);
	connect(m_worker, &ExtractWorker::progress, this, &MainWindow::onFileProgress);
	connect(m_worker, &ExtractWorker::finished, this, &MainWindow::onExtractionFinished);
	m_worker->start();
}

void MainWindow::onCancelExtraction() {
	if (m_worker && m_worker->isRunning()) {
		// Disable cancel button immediately to prevent multiple clicks
		m_cancelBtn->setEnabled(false);
		m_cancelBtn->setText(tr("Canceling..."));
		m_logArea->append("\nCanceling...");

		m_worker->requestInterruption();
		m_worker->quit();

		// Wait up to 3s, keeping UI responsive
		for (int i = 0; i < 30 && m_worker->isRunning(); ++i) {
			QApplication::processEvents();
			m_worker->wait(100);
		}
		if (m_worker->isRunning()) {
			m_worker->terminate();
			m_worker->wait(1000);
		}

		m_logArea->append("Canceled by user.");
		m_worker = nullptr;

		// Brief cooldown before re-enabling Extract All
		m_cancelBtn->setVisible(false);
		m_statusLabel->setText(tr("Canceled"));

		QTimer::singleShot(500, this, [this]() {
			setExtractionActive(false);
		});
	}
}

void MainWindow::onExtractionLog(const QString &message) {
	m_logArea->append(message);
	QScrollBar *scrollBar = m_logArea->verticalScrollBar();
	scrollBar->setValue(scrollBar->maximum());
}

void MainWindow::onCopyLog() {
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(m_logArea->toPlainText());
	m_statusLabel->setText(tr("Log copied to clipboard!"));
}

void MainWindow::onClearLog() {
	m_logArea->clear();
}

void MainWindow::updateLogButtons() {
	bool hasContent = !m_logArea->toPlainText().trimmed().isEmpty();
	m_copyLogBtn->setEnabled(hasContent);
	m_clearLogBtn->setEnabled(hasContent);
}

// ========================================
// === shadPS4 Auto-Detection
// ========================================

void MainWindow::onDetectShadPS4() {
	ShadPS4Config config = m_settings->detectShadPS4();

	if (config.found) {
		// Apply detected directories with visual feedback
		if (!config.gamesDir.isEmpty()) {
			m_gamesDirInput->setText(config.gamesDir);
			// Highlight field briefly
			m_gamesDirInput->setStyleSheet("QLineEdit { background-color: rgba(34, 197, 94, 0.3); }");
			QTimer::singleShot(1500, m_gamesDirInput, [this]() {
				m_gamesDirInput->setStyleSheet("");
			});
		}
		if (!config.addonsDir.isEmpty()) {
			m_addonsDirInput->setText(config.addonsDir);
			// Highlight field briefly
			m_addonsDirInput->setStyleSheet("QLineEdit { background-color: rgba(34, 197, 94, 0.3); }");
			QTimer::singleShot(1500, m_addonsDirInput, [this]() {
				m_addonsDirInput->setStyleSheet("");
			});
		}

		m_logArea->append("======================================");
		m_logArea->append(tr("shadPS4 detected!"));
		m_logArea->append(tr("Config: %1").arg(config.configPath));
		if (!config.gamesDir.isEmpty()) {
			m_logArea->append(tr("Games: %1").arg(config.gamesDir));
		}
		if (!config.addonsDir.isEmpty()) {
			m_logArea->append(tr("DLCs: %1").arg(config.addonsDir));
		}
		m_logArea->append("======================================");

		// Auto-save settings on successful detection
		m_settings->setGamesDir(m_gamesDirInput->text());
		m_settings->setAddonsDir(m_addonsDirInput->text());
		m_statusLabel->setText(tr("<span style='color:#22c55e;'>✔</span> shadPS4 detected! Directories configured and saved."));
	} else {
		m_statusLabel->setText(tr("<span style='color:#ef4444;'>✘</span> shadPS4 not found. Configure directories manually."));
		m_logArea->append(tr("shadPS4 config.json not found."));
		m_logArea->append(tr("Expected locations:"));
		#ifdef Q_OS_LINUX
		m_logArea->append(tr("  ~/.local/share/shadPS4/config.json"));
		#elif defined(Q_OS_WIN)
		m_logArea->append(tr("  %%APPDATA%%/shadPS4/config.json"));
		#elif defined(Q_OS_MAC)
		m_logArea->append(tr("  ~/Library/Application Support/shadPS4/config.json"));
		#endif
	}
}

void MainWindow::onOverallProgress(int percent) {
	m_progressBar->setValue(percent);
	m_progressBar->setProperty("complete", percent >= 100);
	m_progressBar->style()->unpolish(m_progressBar);
	m_progressBar->style()->polish(m_progressBar);
}

void MainWindow::onBatchProgress(int currentFile, int totalFiles) {
	m_batchCurrentFile = currentFile;
	m_batchTotalFiles = totalFiles;
	m_statusLabel->setText(tr("Extracting file %1/%2...").arg(currentFile).arg(totalFiles));
}

void MainWindow::onFileProgress(int current, int total) {
	if (m_batchTotalFiles > 0) {
		m_statusLabel->setText(tr("Extracting file %1/%2 (%3/%4)...").arg(m_batchCurrentFile).arg(m_batchTotalFiles).arg(current).arg(total));
	} else {
		m_statusLabel->setText(tr("Extracting file %1/%2...").arg(current).arg(total));
	}
}

void MainWindow::onExtractionFinished(int returnCode) {
	m_batchCurrentFile = 0;
	m_batchTotalFiles = 0;

	m_logArea->append("");
	m_logArea->append("======================================");

	if (returnCode == 0) {
		m_progressBar->setValue(100);
		m_progressBar->setProperty("complete", true);
		m_progressBar->style()->unpolish(m_progressBar);
		m_progressBar->style()->polish(m_progressBar);
		m_logArea->append("Completed successfully!");
		m_statusLabel->setText(tr("Done!"));
	} else {
		m_logArea->append(QString("Error (code %1)").arg(returnCode));
		m_statusLabel->setText(QString(tr("Failed (code %1)")).arg(returnCode));
	}

	m_logArea->append("======================================");
	setExtractionActive(false);
}

// ========================================
// === Settings
// ========================================

void MainWindow::onSaveSettings() {
	m_settings->setGamesDir(m_gamesDirInput->text());
	m_settings->setAddonsDir(m_addonsDirInput->text());
	m_settings->setLanguage(m_languageCombo->currentData().toString());
	m_statusLabel->setText(tr("<span style='color:#22c55e;'>✔</span> Settings saved!"));
}

// ========================================
// === Language
// ========================================

static QString findTranslationsDir() {
	QString appDir = QApplication::applicationDirPath();

	// Possible locations (in priority order):
	// 1. Installed/AppImage: <appDir>/../share/pkgunbox/translations/
	// 2. Dev build (flat):   <appDir>/translations/
	// 3. Alongside binary:   <appDir>/pkgunbox_translations/ (portable)
	QStringList candidates = {
		appDir + "/../share/pkgunbox/translations",
		appDir + "/translations",
		appDir + "/pkgunbox_translations",
	};

	for (const QString &path : candidates) {
		if (QFileInfo::exists(path)) {
			return path;
		}
	}
	return {};
}

void MainWindow::onLanguageChanged(int index) {
	QString lang = m_languageCombo->itemData(index).toString();
	m_settings->setLanguage(lang);

	static QTranslator *translator = nullptr;
	if (!translator) {
		translator = new QTranslator(qApp);
	}

	// Remove previous translation
	qApp->removeTranslator(translator);

	// English is the source language — no translation needed
	if (lang == "en") {
		retranslateUI();
		return;
	}

	// Find translations directory
	QString qmPath = findTranslationsDir();
	if (qmPath.isEmpty()) {
		qWarning() << "Translations directory not found";
		retranslateUI();
		return;
	}

	QString qmFile = qmPath + "/pkgunbox_" + lang + ".qm";

	// Try loading from filesystem
	if (translator->load(qmFile)) {
		qApp->installTranslator(translator);
	} else {
		// Fallback: try from Qt resources (embedded in binary)
		if (translator->load(":/translations/pkgunbox_" + lang)) {
			qApp->installTranslator(translator);
		} else {
			qWarning() << "Failed to load translation:" << qmFile;
		}
	}

	retranslateUI();
}

void MainWindow::retranslateUI() {
	setWindowTitle(QString("PKGUnbox - v%1").arg(PKGUNBOX_VERSION));
	m_dropText->setText(tr("Drag .pkg files here"));
	m_dropSubtext->setText(tr("or click to browse (multiple files supported)"));
	m_fileCountLabel->setText(tr("Selected Files (%1)").arg(m_selectedFiles.size()));
	m_addFilesBtn->setText(tr("+ Add"));
	m_addFilesBtn->setToolTip(tr("Add more PKG files"));
	m_clearFilesBtn->setText(tr("Clear All"));
	m_clearFilesBtn->setToolTip(tr("Clear all selected files"));

	// Dir section
	m_dirTitleLabel->setText(tr("Destination"));
	m_lblGames->setText(tr("Games:"));
	m_lblAddons->setText(tr("DLCs:"));
	m_saveBtn->setText(tr("Save"));
	m_saveBtn->setToolTip(tr("Click to save destination settings"));
	m_selectGamesDirBtn->setToolTip(tr("Click to select games directory"));
	m_selectAddonsDirBtn->setToolTip(tr("Click to select DLCs directory"));
	m_detectShadPS4Btn->setText(tr("Auto-detect shadPS4"));
	m_detectShadPS4Btn->setToolTip(tr("Click to auto-detect shadPS4 installation and configure directories"));

	m_extractBtn->setText(tr("Extract All"));
	m_extractBtn->setToolTip(tr("Start batch extraction"));
	m_cancelBtn->setText(tr("Cancel"));

	m_logTitleLabel->setText(tr("Log"));
	m_copyLogBtn->setText(tr("Copy Log"));
	m_copyLogBtn->setToolTip(tr("Copy log contents to clipboard"));
	m_clearLogBtn->setText(tr("Clear Log"));
	m_clearLogBtn->setToolTip(tr("Clear log contents"));
	m_langLabel->setText(tr("Language:"));
	m_logArea->setPlaceholderText(tr("Waiting for operation..."));

	// Update status if ready
	if (!m_extractBtn->isEnabled() && !m_worker) {
		m_statusLabel->setText(tr("Ready"));
	}
}

// ========================================
// === Event Filter (drop zone click)
// ========================================

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
	if (obj == m_dropZone && event->type() == QEvent::MouseButtonRelease) {
		onSelectFiles();
		return true;
	}
	return QMainWindow::eventFilter(obj, event);
}

// ========================================
// === Drag & Drop
// ========================================

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
	if (event->mimeData()->hasUrls()) {
		for (const auto &url : event->mimeData()->urls()) {
			if (url.toLocalFile().toLower().endsWith(".pkg")) {
				event->acceptProposedAction();
				return;
			}
		}
	}
}

void MainWindow::dropEvent(QDropEvent *event) {
	QStringList pkgFiles;
	for (const auto &url : event->mimeData()->urls()) {
		QString path = url.toLocalFile();
		if (path.toLower().endsWith(".pkg")) {
			pkgFiles.append(path);
		}
	}
	if (!pkgFiles.isEmpty()) {
		addFilesToList(pkgFiles);
	}
}
