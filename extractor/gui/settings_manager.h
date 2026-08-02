#pragma once

#include <QObject>
#include <QSettings>

/**
 * Result of shadPS4 auto-detection
 */
struct ShadPS4Config {
	bool found;           // Whether config.json was found
	QString gamesDir;     // First enabled path from install_dirs array
	QString addonsDir;    // addon_install_dir from config
	QString configPath;   // Path to config.json that was read
};

class SettingsManager : public QObject {
	Q_OBJECT

public:
	explicit SettingsManager(QObject *parent = nullptr);

	QString gamesDir() const;
	void setGamesDir(const QString &dir);

	QString addonsDir() const;
	void setAddonsDir(const QString &dir);

	QString language() const;
	void setLanguage(const QString &lang);

	/**
	 * Detect shadPS4 installation by reading config.json
	 * @return ShadPS4Config with found status and directories
	 */
	ShadPS4Config detectShadPS4() const;

private:
	/**
	 * Get the expected path to shadPS4 config.json for current OS
	 */
	QString shadPS4ConfigPath() const;

	QSettings *m_settings;
};
