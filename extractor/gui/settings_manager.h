#pragma once

#include <QObject>
#include <QSettings>

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

private:
	QSettings *m_settings;
};
