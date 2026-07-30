#include "extract_worker.h"
#include <QProcess>
#include <QCoreApplication>
#include <QRegularExpression>

ExtractWorker::ExtractWorker(const QString &pkgPath, const QString &gamesDir, const QString &addonsDir, QObject *parent)
	: QThread(parent)
	, m_pkgPath(pkgPath)
	, m_gamesDir(gamesDir)
	, m_addonsDir(addonsDir)
{}

void ExtractWorker::run() {
	QString cliPath = QCoreApplication::applicationDirPath() + "/pkgunbox";

	emit log(QString(">>> Checking type: %1").arg(m_pkgPath));

	QProcess checkType;
	checkType.start(cliPath, {m_pkgPath, "--check-type"});
	checkType.waitForFinished(60000);

	int ret = checkType.exitCode();
	emit log(QString("    Code: %1").arg(ret));

	if (ret == 0) {
		emit log("!!! Error: not a valid PKG.");
		emit finished(-1);
		return;
	}

	if (ret == 101) {
		emit log(">>> Type: Base game");
	} else if (ret == 102) {
		emit log(">>> Type: Update");
	} else if (ret == 103) {
		emit log(">>> Type: DLC");
	}

	// Route to correct directory based on type
	QString extractDir = m_gamesDir;
	if (ret == 103 && !m_addonsDir.isEmpty()) {
		extractDir = m_addonsDir;
		emit log(QString(">>> DLC detected, using addons directory: %1").arg(extractDir));
	}

	emit log(QString(">>> Extracting to: %1").arg(extractDir));

	QProcess extract;
	extract.setProcessChannelMode(QProcess::MergedChannels);
	extract.start(cliPath, {m_pkgPath, extractDir});

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
			} else {
				emit log("    " + line);
			}
		}

		if (!buffer.isEmpty()) {
			QRegularExpressionMatch match = progressRegex.match(buffer);
			if (match.hasMatch()) {
				int current = match.captured(1).toInt();
				int total = match.captured(2).toInt();
				emit progress(current, total);
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
			emit log("    " + line.trimmed());
		}
	}

	int finalRet = extract.exitCode();

	if (finalRet == 0) {
		emit log(">>> Extraction completed successfully!");
	} else {
		emit log(QString("!!! Error (code %1)").arg(finalRet));
	}

	emit finished(finalRet);
}
