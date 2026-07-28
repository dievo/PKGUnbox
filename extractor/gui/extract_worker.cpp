#include "extract_worker.h"
#include <QProcess>
#include <QCoreApplication>
#include <QRegularExpression>

ExtractWorker::ExtractWorker(const QString &pkgPath, const QString &destDir, QObject *parent)
	: QThread(parent)
	, m_pkgPath(pkgPath)
	, m_destDir(destDir)
{}

void ExtractWorker::run() {
	QString cliPath = QCoreApplication::applicationDirPath() + "/pkgunbox";

	emit log(QString(">>> Verificando tipo: %1").arg(m_pkgPath));

	QProcess checkType;
	checkType.start(cliPath, {m_pkgPath, "--check-type"});
	checkType.waitForFinished(60000);

	int ret = checkType.exitCode();
	emit log(QString("    Código: %1").arg(ret));

	if (ret == 0) {
		emit log("!!! Erro: não é um PKG válido.");
		emit finished(-1);
		return;
	}

	if (ret == 101) {
		emit log(">>> Tipo: Jogo base");
	} else if (ret == 102) {
		emit log(">>> Tipo: Atualização");
	} else if (ret == 103) {
		emit log(">>> Tipo: DLC");
	}

	emit log(QString(">>> Extraindo para: %1").arg(m_destDir));

	QProcess extract;
	extract.setProcessChannelMode(QProcess::MergedChannels);
	extract.start(cliPath, {m_pkgPath, m_destDir});

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
		emit log(">>> Extração concluída com sucesso!");
	} else {
		emit log(QString("!!! Erro (código %1)").arg(finalRet));
	}

	emit finished(finalRet);
}
