#include "extract_worker.h"
#include <QProcess>
#include <QCoreApplication>

ExtractWorker::ExtractWorker(const QString &pkgPath, const QString &destDir, QObject *parent)
	: QThread(parent)
	, m_pkgPath(pkgPath)
	, m_destDir(destDir)
{}

void ExtractWorker::run() {
	QString cliPath = QCoreApplication::applicationDirPath() + "/pkg_extractor";

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

	QString dest = m_destDir;
	if (ret == 101) {
		emit log(">>> Tipo: Jogo base");
	} else if (ret == 102) {
		emit log(">>> Tipo: Atualização");
	} else if (ret == 103) {
		emit log(">>> Tipo: DLC");
	}

	emit log(QString(">>> Extraindo para: %1").arg(dest));

	QProcess extract;
	extract.setProcessChannelMode(QProcess::MergedChannels);
	extract.start(cliPath, {m_pkgPath, dest});

	while (extract.canReadLine()) {
		emit log("    " + QString::fromUtf8(extract.readLine()));
	}

	extract.waitForFinished(-1);
	int finalRet = extract.exitCode();

	if (finalRet == 0) {
		emit log(">>> Extração concluída com sucesso!");
	} else {
		emit log(QString("!!! Erro (código %1)").arg(finalRet));
	}

	emit finished(finalRet);
}
