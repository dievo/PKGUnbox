#include <QApplication>
#include <QFile>
#include "main_window.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	app.setApplicationName("PKGUnbox");
	app.setOrganizationName("PKGUnbox");

	// Load dark theme
	QFile styleFile(":/styles/dark_theme.qss");
	if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		app.setStyleSheet(styleFile.readAll());
		styleFile.close();
	}

	MainWindow window;
	window.show();

	return app.exec();
}
