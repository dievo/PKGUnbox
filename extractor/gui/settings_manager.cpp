#include "settings_manager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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
	// shadPS4 stores config as config.json (not TOML)
	#ifdef Q_OS_LINUX
	// Linux: ~/.local/share/shadPS4/config.json (XDG_DATA_HOME)
	QString dataDir = QDir::homePath() + "/.local/share/shadPS4";
	// Fallback: respect XDG_DATA_HOME if set
	QString xdgData = qEnvironmentVariable("XDG_DATA_HOME");
	if (!xdgData.isEmpty()) {
		dataDir = xdgData + "/shadPS4";
	}
	return dataDir + "/config.json";
	#elif defined(Q_OS_WIN)
	// Windows: %APPDATA%/shadPS4/config.json (CSIDL_APPDATA = Roaming)
	QString appData = qEnvironmentVariable("APPDATA");
	if (!appData.isEmpty()) {
		return appData + "/shadPS4/config.json";
	}
	return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
		+ "/../shadPS4/config.json";
	#elif defined(Q_OS_MAC)
	// macOS: ~/Library/Application Support/shadPS4/config.json
	return QDir::homePath() + "/Library/Application Support/shadPS4/config.json";
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

	// Parse the JSON config file
	QFile file(configPath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return result;
	}

	QByteArray data = file.readAll();
	file.close();

	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
	if (parseError.error != QJsonParseError::NoError || doc.isNull()) {
		return result;
	}

	QJsonObject root = doc.object();
	QJsonObject general = root.value("General").toObject();

	// Extract games directory from install_dirs array
	// Structure: "install_dirs": [{"enabled": true, "path": "..."}]
	QJsonArray installDirs = general.value("install_dirs").toArray();
	for (const QJsonValue &entry : installDirs) {
		QJsonObject obj = entry.toObject();
		if (obj.value("enabled").toBool()) {
			QString path = obj.value("path").toString();
			if (!path.isEmpty()) {
				result.gamesDir = path;
				break;
			}
		}
	}

	// Extract addons directory (simple string key)
	result.addonsDir = general.value("addon_install_dir").toString();

	// Check if at least one directory was found
	if (!result.gamesDir.isEmpty() || !result.addonsDir.isEmpty()) {
		result.found = true;
		result.configPath = configPath;
	}

	return result;
}
