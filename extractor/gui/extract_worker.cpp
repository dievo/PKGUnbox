#include "extract_worker.h"
#include <QProcess>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QFileInfo>

ExtractWorker::ExtractWorker(const QString &pkgPath, const QString &gamesDir, const QString &addonsDir, QObject *parent)
	: QThread(parent)
	, m_pkgPaths(QStringList() << pkgPath)
	, m_gamesDir(gamesDir)
	, m_addonsDir(addonsDir)
{}

ExtractWorker::ExtractWorker(const QStringList &pkgPaths, const QString &gamesDir, const QString &addonsDir, QObject *parent)
	: QThread(parent)
	, m_pkgPaths(pkgPaths)
	, m_gamesDir(gamesDir)
	, m_addonsDir(addonsDir)
{}

void ExtractWorker::run() {
	int totalFiles = m_pkgPaths.size();
	int successCount = 0;
	int failCount = 0;
	m_batchTotalFiles = totalFiles;
	m_batchCompletedFiles = 0;

	if (totalFiles == 0) {
		emit log("!!! No files to extract.");
		emit finished(-1);
		return;
	}

	// Single file mode
	if (totalFiles == 1) {
		int ret = extractSingle(m_pkgPaths.first(), m_gamesDir, m_addonsDir);
		emit finished(ret);
		return;
	}

	// Batch mode
	emit log(QString(">>> Batch extraction: %1 file(s)").arg(totalFiles));
	emit log("======================================");

	for (int i = 0; i < totalFiles; ++i) {
		// Check if thread was requested to stop
		if (isInterruptionRequested()) {
			emit log("\n>>> Batch extraction canceled by user.");
			break;
		}

		m_batchCompletedFiles = i;

		emit log(QString("\n>>> [%1/%2] Processing: %3")
			.arg(i + 1)
			.arg(totalFiles)
			.arg(QFileInfo(m_pkgPaths[i]).fileName()));

		// Emit batch progress
		emit batchProgress(i + 1, totalFiles);

		int ret = extractSingle(m_pkgPaths[i], m_gamesDir, m_addonsDir);

		if (ret == 0) {
			successCount++;
		} else {
			failCount++;
		}
	}

	emit log("\n======================================");
	emit log(QString(">>> Batch complete: %1 succeeded, %2 failed")
		.arg(successCount)
		.arg(failCount));
	emit log("======================================");

	// Return 0 if all succeeded, 1 if any failed
	emit finished(failCount == 0 ? 0 : 1);
}

int ExtractWorker::extractSingle(const QString &pkgPath, const QString &gamesDir, const QString &addonsDir) {
	QString cliPath = QCoreApplication::applicationDirPath() + "/pkgunbox";

	emit log(QString("    Checking type: %1").arg(QFileInfo(pkgPath).fileName()));

	QProcess checkType;
	checkType.start(cliPath, {pkgPath, "--check-type"});
	checkType.waitForFinished(60000);

	int ret = checkType.exitCode();
	emit log(QString("    Type code: %1").arg(ret));

	if (ret == 0) {
		emit log("    !!! Error: not a valid PKG.");
		return -1;
	}

	QString typeName;
	if (ret == 101) {
		typeName = "Base Game";
	} else if (ret == 102) {
		typeName = "Update";
	} else if (ret == 103) {
		typeName = "DLC";
	}
	emit log(QString("    Type: %1").arg(typeName));

	// Route to correct directory based on type
	QString extractDir = gamesDir;
	if (ret == 103 && !addonsDir.isEmpty()) {
		extractDir = addonsDir;
		emit log(QString("    DLC detected, using addons directory: %1").arg(extractDir));
	}

	emit log(QString("    Extracting to: %1").arg(extractDir));

	QProcess extract;
	extract.setProcessChannelMode(QProcess::MergedChannels);
	extract.start(cliPath, {pkgPath, extractDir});

	QRegularExpression progressRegex(R"(Extracting file (\d+) of (\d+))");
	QString buffer;

	while (extract.state() != QProcess::NotRunning) {
		extract.waitForReadyRead(100);

		QByteArray data = extract.readAllStandardOutput();
		if (data.isEmpty()) continue;

		buffer += QString::fromUtf8(data);

		while (true) {
			int nlIdx = buffer.indexOf('\n');
			if (nlIdx < 0) break;

			QString line = buffer.left(nlIdx).trimmed();
			buffer = buffer.mid(nlIdx + 1);

			if (line.isEmpty()) continue;

			QRegularExpressionMatch match = progressRegex.match(line);
			if (match.hasMatch()) {
				int current = match.captured(1).toInt();
				int total = match.captured(2).toInt();
				emit progress(current, total);
				if (m_batchTotalFiles > 0 && total > 0) {
					double fileProgress = static_cast<double>(current) / total;
					int overall = static_cast<int>((m_batchCompletedFiles + fileProgress) / m_batchTotalFiles * 100.0);
					emit overallProgress(qBound(0, overall, 100));
				}
			} else {
				emit log("        " + line);
			}
		}

		if (!buffer.isEmpty()) {
			QRegularExpressionMatch match = progressRegex.match(buffer);
			if (match.hasMatch()) {
				int current = match.captured(1).toInt();
				int total = match.captured(2).toInt();
				emit progress(current, total);
				if (m_batchTotalFiles > 0 && total > 0) {
					double fileProgress = static_cast<double>(current) / total;
					int overall = static_cast<int>((m_batchCompletedFiles + fileProgress) / m_batchTotalFiles * 100.0);
					emit overallProgress(qBound(0, overall, 100));
				}
				buffer.clear();
			}
		}
	}

	extract.waitForFinished(5000);

	QByteArray remaining = extract.readAllStandardOutput();
	if (!remaining.isEmpty()) {
		buffer += QString::fromUtf8(remaining);
		QStringList lines = buffer.split('\n', Qt::SkipEmptyParts);
		for (const QString &line : lines) {
			emit log("        " + line.trimmed());
		}
	}

	int finalRet = extract.exitCode();

	if (finalRet == 0) {
		emit log("    ✓ Extraction completed successfully!");
	} else {
		emit log(QString("    ✗ Error (code %1)").arg(finalRet));
	}

	return finalRet;
}
