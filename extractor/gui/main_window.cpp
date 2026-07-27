#include "main_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFont>
#include <QFileInfo>

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
		m_worker->wait();
	}
}

void MainWindow::setupUI() {
	setWindowTitle("PKG Extractor - ShadPs4Plus");
	setMinimumSize(700, 520);
	setAcceptDrops(true);

	auto *central = new QWidget(this);
	setCentralWidget(central);
	auto *layout = new QVBoxLayout(central);

	// === Arquivo PKG ===
	auto *fileGroup = new QGroupBox("Arquivo PKG");
	auto *fileLayout = new QHBoxLayout();

	m_fileInput = new QLineEdit();
	m_fileInput->setPlaceholderText("Arraste um .pkg aqui ou clique em Selecionar...");
	m_fileInput->setReadOnly(true);
	fileLayout->addWidget(m_fileInput);

	auto *btnSelect = new QPushButton("Selecionar");
	btnSelect->setFixedWidth(100);
	connect(btnSelect, &QPushButton::clicked, this, &MainWindow::onSelectFile);
	fileLayout->addWidget(btnSelect);

	fileGroup->setLayout(fileLayout);
	layout->addWidget(fileGroup);

	// === Diretórios ===
	auto *dirGroup = new QGroupBox("Diretórios de Destino");
	auto *dirLayout = new QFormLayout();

	m_gamesDirInput = new QLineEdit();
	auto *btnGames = new QPushButton("...");
	btnGames->setFixedWidth(36);
	connect(btnGames, &QPushButton::clicked, this, &MainWindow::onSelectGamesDir);
	auto *rowGames = new QHBoxLayout();
	rowGames->addWidget(m_gamesDirInput);
	rowGames->addWidget(btnGames);
	dirLayout->addRow("Jogos:", rowGames);

	m_addonsDirInput = new QLineEdit();
	auto *btnAddons = new QPushButton("...");
	btnAddons->setFixedWidth(36);
	connect(btnAddons, &QPushButton::clicked, this, &MainWindow::onSelectAddonsDir);
	auto *rowAddons = new QHBoxLayout();
	rowAddons->addWidget(m_addonsDirInput);
	rowAddons->addWidget(btnAddons);
	dirLayout->addRow("DLCs/Updates:", rowAddons);

	dirGroup->setLayout(dirLayout);
	layout->addWidget(dirGroup);

	// === Botões ===
	auto *btnLayout = new QHBoxLayout();
	m_extractBtn = new QPushButton("Extrair");
	m_extractBtn->setFixedHeight(40);
	m_extractBtn->setEnabled(false);
	connect(m_extractBtn, &QPushButton::clicked, this, &MainWindow::onStartExtraction);
	btnLayout->addWidget(m_extractBtn);

	auto *btnSave = new QPushButton("Salvar Config");
	btnSave->setFixedHeight(40);
	btnSave->setFixedWidth(120);
	connect(btnSave, &QPushButton::clicked, this, &MainWindow::onSaveSettings);
	btnLayout->addWidget(btnSave);
	layout->addLayout(btnLayout);

	// === Progresso ===
	m_progressBar = new QProgressBar();
	m_progressBar->setVisible(false);
	layout->addWidget(m_progressBar);

	// === Log ===
	auto *logGroup = new QGroupBox("Log");
	auto *logLayout = new QVBoxLayout();
	m_logArea = new QTextEdit();
	m_logArea->setReadOnly(true);
	m_logArea->setFont(QFont("Monospace", 9));
	logLayout->addWidget(m_logArea);
	logGroup->setLayout(logLayout);
	layout->addWidget(logGroup);

	// === Status ===
	m_statusLabel = new QLabel("Pronto");
	layout->addWidget(m_statusLabel);
}

void MainWindow::loadSettings() {
	m_gamesDirInput->setText(m_settings->gamesDir());
	m_addonsDirInput->setText(m_settings->addonsDir());
}

void MainWindow::onSelectFile() {
	QString path = QFileDialog::getOpenFileName(
		this, "Selecionar arquivo PKG", "",
		"Arquivos PKG (*.pkg);;Todos os arquivos (*)"
	);
	if (!path.isEmpty()) {
		m_fileInput->setText(path);
		m_extractBtn->setEnabled(true);
		m_statusLabel->setText(QString("Arquivo: %1").arg(QFileInfo(path).fileName()));
	}
}

void MainWindow::onSelectGamesDir() {
	QString dir = QFileDialog::getExistingDirectory(this, "Selecionar diretório de jogos", m_gamesDirInput->text());
	if (!dir.isEmpty()) {
		m_gamesDirInput->setText(dir);
	}
}

void MainWindow::onSelectAddonsDir() {
	QString dir = QFileDialog::getExistingDirectory(this, "Selecionar diretório de DLCs/Updates", m_addonsDirInput->text());
	if (!dir.isEmpty()) {
		m_addonsDirInput->setText(dir);
	}
}

void MainWindow::onStartExtraction() {
	QString pkgPath = m_fileInput->text();
	if (pkgPath.isEmpty()) {
		QMessageBox::warning(this, "Aviso", "Selecione um arquivo PKG.");
		return;
	}

	QString gamesDir = m_gamesDirInput->text();
	if (gamesDir.isEmpty()) {
		QMessageBox::warning(this, "Aviso", "Configure o diretório de jogos.");
		return;
	}

	m_extractBtn->setEnabled(false);
	m_progressBar->setVisible(true);
	m_logArea->clear();

	m_worker = new ExtractWorker(pkgPath, gamesDir, this);
	connect(m_worker, &ExtractWorker::log, this, &MainWindow::onExtractionLog);
	connect(m_worker, &ExtractWorker::finished, this, &MainWindow::onExtractionFinished);
	m_worker->start();
}

void MainWindow::onExtractionLog(const QString &message) {
	m_logArea->append(message);
}

void MainWindow::onExtractionFinished(int returnCode) {
	m_extractBtn->setEnabled(true);
	m_progressBar->setVisible(false);

	if (returnCode == 0) {
		m_statusLabel->setText("Extração concluída!");
	} else {
		m_statusLabel->setText(QString("Falha (código %1)").arg(returnCode));
	}
}

void MainWindow::onSaveSettings() {
	m_settings->setGamesDir(m_gamesDirInput->text());
	m_settings->setAddonsDir(m_addonsDirInput->text());
	QMessageBox::information(this, "Sucesso", "Configurações salvas!");
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
	if (event->mimeData()->hasUrls()) {
		for (const auto &url : event->mimeData()->urls()) {
			if (url.toLocalFile().endsWith(".pkg")) {
				event->acceptProposedAction();
				return;
			}
		}
	}
}

void MainWindow::dropEvent(QDropEvent *event) {
	for (const auto &url : event->mimeData()->urls()) {
		QString path = url.toLocalFile();
		if (path.endsWith(".pkg")) {
			m_fileInput->setText(path);
			m_extractBtn->setEnabled(true);
			return;
		}
	}
}
