#pragma once

#include <QThread>
#include <QString>

class ExtractWorker : public QThread {
	Q_OBJECT

public:
	explicit ExtractWorker(const QString &pkgPath, const QString &gamesDir, const QString &addonsDir, QObject *parent = nullptr);
	void run() override;

signals:
	void log(const QString &message);
	void progress(int current, int total);
	void finished(int returnCode);

private:
	QString m_pkgPath;
	QString m_gamesDir;
	QString m_addonsDir;
};
