#pragma once

#include <QThread>
#include <QString>
#include <QStringList>

class ExtractWorker : public QThread {
	Q_OBJECT

public:
	// Single file constructor (for backward compatibility)
	explicit ExtractWorker(const QString &pkgPath, const QString &gamesDir, const QString &addonsDir, QObject *parent = nullptr);

	// Multi-file constructor (batch extraction)
	explicit ExtractWorker(const QStringList &pkgPaths, const QString &gamesDir, const QString &addonsDir, QObject *parent = nullptr);

	void run() override;

signals:
	void log(const QString &message);
	void progress(int current, int total);
	void batchProgress(int currentFile, int totalFiles);
	void finished(int returnCode);

private:
	// Extract a single PKG file
	int extractSingle(const QString &pkgPath, const QString &gamesDir, const QString &addonsDir);

	QStringList m_pkgPaths;
	QString m_gamesDir;
	QString m_addonsDir;
};
