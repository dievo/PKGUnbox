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
#include <QScrollBar>
#include <QSizePolicy>
#include <QPainter>
#include <QPainterPath>
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
	setWindowTitle("PKGUnbox");
	setMinimumSize(640, 480);
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
	QColor colGames("#9ece6a");      // Green — games
	QColor colAddons("#bb9af7");     // Purple — addons/DLCs
	QColor colExtract("#5865f2");    // Blurple — extract (primary)
	QColor colSave("#9ece6a");       // Green — save (positive)
	QColor colCancel("#f7768e");     // Red — cancel/danger

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
	m_dropZone->setFixedHeight(90);
	m_dropZone->setCursor(Qt::PointingHandCursor);
	m_dropZone->installEventFilter(this);

	auto *dropLayout = new QVBoxLayout(m_dropZone);
	dropLayout->setAlignment(Qt::AlignCenter);
	dropLayout->setSpacing(2);

	m_dropIcon = new QLabel();
	m_dropIcon->setPixmap(makeIcon("file", colFile).pixmap(32, 32));
	m_dropIcon->setAlignment(Qt::AlignCenter);
	dropLayout->addWidget(m_dropIcon);

	m_dropText = new QLabel("Arraste um arquivo .pkg aqui");
	m_dropText->setObjectName("dropText");
	m_dropText->setAlignment(Qt::AlignCenter);
	dropLayout->addWidget(m_dropText);

	m_dropSubtext = new QLabel("ou clique para selecionar");
	m_dropSubtext->setObjectName("dropSubtext");
	m_dropSubtext->setAlignment(Qt::AlignCenter);
	dropLayout->addWidget(m_dropSubtext);

	mainLayout->addWidget(m_dropZone);

	// ========================================
	// === File Selected (hidden by default)
	// ========================================
	m_fileDisplay = new QWidget();
	m_fileDisplay->setVisible(false);
	auto *fileRow = new QHBoxLayout(m_fileDisplay);
	fileRow->setContentsMargins(0, 0, 0, 0);
	fileRow->setSpacing(6);

	m_fileInput = new QLineEdit();
	m_fileInput->setReadOnly(true);
	m_fileInput->setMinimumHeight(32);
	m_fileInput->setPlaceholderText("Nenhum arquivo selecionado");
	fileRow->addWidget(m_fileInput, 1);

	m_clearFileBtn = new QPushButton();
	m_clearFileBtn->setIcon(makeIcon("cancel", colCancel));
	m_clearFileBtn->setObjectName("clearFileBtn");
	m_clearFileBtn->setFixedSize(28, 28);
	m_clearFileBtn->setToolTip("Limpar arquivo selecionado");
	connect(m_clearFileBtn, &QPushButton::clicked, this, &MainWindow::onClearFile);
	fileRow->addWidget(m_clearFileBtn, 0);

	mainLayout->addWidget(m_fileDisplay);

	// ========================================
	// === Diretorios de Destino + Salvar
	// ========================================
	auto *dirContainer = new QWidget();
	dirContainer->setObjectName("dirContainer");
	auto *dirOuter = new QVBoxLayout(dirContainer);
	dirOuter->setContentsMargins(0, 0, 0, 0);
	dirOuter->setSpacing(0);

	// Title bar with Save button
	auto *dirTitleBar = new QHBoxLayout();
	dirTitleBar->setContentsMargins(10, 6, 10, 2);
	auto *dirTitleLabel = new QLabel("Destino");
	dirTitleLabel->setObjectName("groupTitle");
	dirTitleBar->addWidget(dirTitleLabel);
	dirTitleBar->addStretch();

	m_saveBtn = new QPushButton();
	m_saveBtn->setIcon(makeIcon("settings", colSave));
	m_saveBtn->setText("Salvar");
	m_saveBtn->setObjectName("saveBtn");
	m_saveBtn->setFixedHeight(24);
	m_saveBtn->setToolTip("Salvar as configuracoes de destino");
	connect(m_saveBtn, &QPushButton::clicked, this, &MainWindow::onSaveSettings);
	dirTitleBar->addWidget(m_saveBtn);

	dirOuter->addLayout(dirTitleBar);

	// Grid for directory inputs
	auto *dirGrid = new QGridLayout();
	dirGrid->setSpacing(8);
	dirGrid->setContentsMargins(10, 4, 10, 10);

	// Games directory
	auto *lblGames = new QLabel("Jogos:");
	lblGames->setFixedWidth(55);
	dirGrid->addWidget(lblGames, 0, 0);

	m_gamesDirInput = new QLineEdit();
	m_gamesDirInput->setMinimumHeight(30);
	dirGrid->addWidget(m_gamesDirInput, 0, 1);

	m_selectGamesDirBtn = new QPushButton();
	m_selectGamesDirBtn->setIcon(makeIcon("folder", colGames));
	m_selectGamesDirBtn->setObjectName("browseBtn");
	m_selectGamesDirBtn->setFixedSize(30, 30);
	m_selectGamesDirBtn->setToolTip("Selecionar diretorio de jogos");
	connect(m_selectGamesDirBtn, &QPushButton::clicked, this, &MainWindow::onSelectGamesDir);
	dirGrid->addWidget(m_selectGamesDirBtn, 0, 2);

	// Addons directory
	auto *lblAddons = new QLabel("DLCs:");
	lblAddons->setFixedWidth(55);
	dirGrid->addWidget(lblAddons, 1, 0);

	m_addonsDirInput = new QLineEdit();
	m_addonsDirInput->setMinimumHeight(30);
	dirGrid->addWidget(m_addonsDirInput, 1, 1);

	m_selectAddonsDirBtn = new QPushButton();
	m_selectAddonsDirBtn->setIcon(makeIcon("folder", colAddons));
	m_selectAddonsDirBtn->setObjectName("browseBtn");
	m_selectAddonsDirBtn->setFixedSize(30, 30);
	m_selectAddonsDirBtn->setToolTip("Selecionar diretorio de DLCs/Updates");
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
	m_extractBtn->setText("Extrair");
	m_extractBtn->setObjectName("extractBtn");
	m_extractBtn->setMinimumSize(160, 40);
	m_extractBtn->setEnabled(false);
	m_extractBtn->setToolTip("Iniciar extracao do PKG");
	connect(m_extractBtn, &QPushButton::clicked, this, &MainWindow::onStartExtraction);
	extractLayout->addWidget(m_extractBtn);

	m_cancelBtn = new QPushButton();
	m_cancelBtn->setIcon(makeIcon("cancel", colCancel));
	m_cancelBtn->setText("Cancelar");
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
	auto *logLabel = new QLabel("Log");
	logLabel->setObjectName("logLabel");
	mainLayout->addWidget(logLabel);

	m_logArea = new QTextEdit();
	m_logArea->setReadOnly(true);
	m_logArea->setPlaceholderText("Aguardando operacao...");
	m_logArea->setMinimumHeight(80);
	mainLayout->addWidget(m_logArea, 1);

	// ========================================
	// === Status Label
	// ========================================
	m_statusLabel = new QLabel("Pronto");
	m_statusLabel->setObjectName("statusLabel");
	mainLayout->addWidget(m_statusLabel);
}

void MainWindow::loadSettings() {
	m_gamesDirInput->setText(m_settings->gamesDir());
	m_addonsDirInput->setText(m_settings->addonsDir());
}

void MainWindow::setExtractionActive(bool active) {
	m_extractBtn->setEnabled(!active);
	m_extractBtn->setVisible(!active);
	m_cancelBtn->setVisible(active);
	m_selectGamesDirBtn->setEnabled(!active);
	m_selectAddonsDirBtn->setEnabled(!active);
	m_saveBtn->setEnabled(!active);
	m_clearFileBtn->setEnabled(!active);

	if (active) {
		m_progressBar->setValue(0);
		m_logArea->clear();
	}
}

void MainWindow::setFileSelected(const QString &path) {
	m_fileInput->setText(path);
	m_fileDisplay->setVisible(true);
	m_dropZone->setVisible(false);
	m_extractBtn->setEnabled(true);
	m_statusLabel->setText(QString("Arquivo: %1").arg(QFileInfo(path).fileName()));
}

void MainWindow::setFileCleared() {
	m_fileInput->clear();
	m_fileDisplay->setVisible(false);
	m_dropZone->setVisible(true);
	m_extractBtn->setEnabled(false);
	m_statusLabel->setText("Pronto");
	m_progressBar->setValue(0);
}

// ========================================
// === File Selection
// ========================================

void MainWindow::onSelectFile() {
	QString path = QFileDialog::getOpenFileName(
		this, "Selecionar arquivo PKG", "",
		"Arquivos PKG (*.pkg);;Todos os arquivos (*)"
	);
	if (!path.isEmpty()) {
		setFileSelected(path);
	}
}

void MainWindow::onClearFile() {
	setFileCleared();
}

void MainWindow::onSelectGamesDir() {
	QString dir = QFileDialog::getExistingDirectory(
		this, "Selecionar diretorio de jogos", m_gamesDirInput->text()
	);
	if (!dir.isEmpty()) {
		m_gamesDirInput->setText(dir);
	}
}

void MainWindow::onSelectAddonsDir() {
	QString dir = QFileDialog::getExistingDirectory(
		this, "Selecionar diretorio de DLCs/Updates", m_addonsDirInput->text()
	);
	if (!dir.isEmpty()) {
		m_addonsDirInput->setText(dir);
	}
}

// ========================================
// === Extraction
// ========================================

void MainWindow::onStartExtraction() {
	QString pkgPath = m_fileInput->text();
	if (pkgPath.isEmpty()) {
		QMessageBox::warning(this, "Aviso", "Selecione um arquivo PKG.");
		return;
	}

	QString gamesDir = m_gamesDirInput->text();
	if (gamesDir.isEmpty()) {
		QMessageBox::warning(this, "Aviso", "Configure o diretorio de jogos.");
		return;
	}

	setExtractionActive(true);

	m_logArea->append("======================================");
	m_logArea->append(QString("Iniciando: %1").arg(QFileInfo(pkgPath).fileName()));
	m_logArea->append(QString("Destino:   %1").arg(gamesDir));
	m_logArea->append("======================================");

	m_worker = new ExtractWorker(pkgPath, gamesDir, this);
	connect(m_worker, &ExtractWorker::log, this, &MainWindow::onExtractionLog);
	connect(m_worker, &ExtractWorker::progress, this, &MainWindow::onExtractionProgress);
	connect(m_worker, &ExtractWorker::finished, this, &MainWindow::onExtractionFinished);
	m_worker->start();
}

void MainWindow::onCancelExtraction() {
	if (m_worker && m_worker->isRunning()) {
		m_logArea->append("\nCancelando...");
		m_worker->quit();
		if (!m_worker->wait(3000)) {
			m_worker->terminate();
			m_worker->wait(1000);
		}
		m_logArea->append("Cancelado pelo usuario.");
		setExtractionActive(false);
		m_statusLabel->setText("Cancelado");
	}
}

void MainWindow::onExtractionLog(const QString &message) {
	m_logArea->append(message);
	QScrollBar *scrollBar = m_logArea->verticalScrollBar();
	scrollBar->setValue(scrollBar->maximum());
}

void MainWindow::onExtractionProgress(int current, int total) {
	if (total > 0) {
		int percent = static_cast<int>((static_cast<double>(current) / total) * 100);
		m_progressBar->setValue(percent);
		m_statusLabel->setText(QString("%1 / %2 (%3%)").arg(current).arg(total).arg(percent));
	}
}

void MainWindow::onExtractionFinished(int returnCode) {
	m_logArea->append("");
	m_logArea->append("======================================");

	if (returnCode == 0) {
		m_progressBar->setValue(100);
		m_logArea->append("Concluido com sucesso!");
		m_statusLabel->setText("Concluido!");
	} else {
		m_logArea->append(QString("Erro (codigo %1)").arg(returnCode));
		m_statusLabel->setText(QString("Falha (codigo %1)").arg(returnCode));
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
	m_statusLabel->setText("Configuracoes salvas!");
}

// ========================================
// === Event Filter (drop zone click)
// ========================================

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
	if (obj == m_dropZone && event->type() == QEvent::MouseButtonRelease) {
		onSelectFile();
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
	for (const auto &url : event->mimeData()->urls()) {
		QString path = url.toLocalFile();
		if (path.toLower().endsWith(".pkg")) {
			setFileSelected(path);
			return;
		}
	}
}
