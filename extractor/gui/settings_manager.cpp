#include "settings_manager.h"
#include <QDir>

SettingsManager::SettingsManager(QObject *parent)
	: QObject(parent)
	, m_settings(new QSettings("PKGUnbox", "PKGUnbox", this))
{}

QString SettingsManager::gamesDir() const {
	return m_settings->value("games_dir", QDir::homePath() + "/Games/PS4").toString();
}

void SettingsManager::setGamesDir(const QString &dir) {
	m_settings->setValue("games_dir", dir);
}

QString SettingsManager::addonsDir() const {
	return m_settings->value("addons_dir", QDir::homePath() + "/Games/PS4/Updates-DLCs").toString();
}

void SettingsManager::setAddonsDir(const QString &dir) {
	m_settings->setValue("addons_dir", dir);
}
