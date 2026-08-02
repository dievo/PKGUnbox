#include "settings_manager.h"
#include "toml_parser.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

SettingsManager::SettingsManager(QObject *parent)
	: QObject(parent)
	, m_settings(new QSettings("PKGUnbox", "PKGUnbox", this))
{}

QString SettingsManager::gamesDir() const {
	return m_settings->value("games_dir", QDir::homePath() + "/Games/PS4/Games").toString();
}

void SettingsManager::setGamesDir(const QString &dir) {
	m_settings->setValue("games_dir", dir);
}

QString SettingsManager::addonsDir() const {
	return m_settings->value("addons_dir", QDir::homePath() + "/Games/PS4/DLCs").toString();
}

void SettingsManager::setAddonsDir(const QString &dir) {
	m_settings->setValue("addons_dir", dir);
}

QString SettingsManager::language() const {
	return m_settings->value("language", "en").toString();
}

void SettingsManager::setLanguage(const QString &lang) {
	m_settings->setValue("language", lang);
}

// ========================================
// === shadPS4 Auto-Detection
// ========================================

QString SettingsManager::shadPS4ConfigPath() const {
	// Expected locations for shadPS4 config.toml
	#ifdef Q_OS_LINUX
	// Linux: ~/.config/shadPS4/config.toml
	return QDir::homePath() + "/.config/shadPS4/config.toml";
	#elif defined(Q_OS_WIN)
	// Windows: %APPDATA%/shadPS4/config.toml
	return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
		+ "/../Roaming/shadPS4/config.toml";
	#elif defined(Q_OS_MAC)
	// macOS: ~/Library/Application Support/shadPS4/config.toml
	return QDir::homePath() + "/Library/Application Support/shadPS4/config.toml";
	#else
	return QString();
	#endif
}

ShadPS4Config SettingsManager::detectShadPS4() const {
	ShadPS4Config result;
	result.found = false;

	QString configPath = shadPS4ConfigPath();
	if (configPath.isEmpty() || !QFileInfo::exists(configPath)) {
		return result;
	}

	// Parse the TOML file
	QMap<QString, QString> config = TomlParser::parseFile(configPath);
	if (config.isEmpty()) {
		return result;
	}

	// Extract directories
	QString gamesDir = TomlParser::getValue(config, "General", "game_install_dir");
	QString addonsDir = TomlParser::getValue(config, "General", "addon_install_dir");

	// Check if at least one directory was found
	if (!gamesDir.isEmpty() || !addonsDir.isEmpty()) {
		result.found = true;
		result.gamesDir = gamesDir;
		result.addonsDir = addonsDir;
		result.configPath = configPath;
	}

	return result;
}
