#include <QApplication>
#include "main_window.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	app.setApplicationName("PKG Extractor");
	app.setOrganizationName("ShadPs4Plus");

	MainWindow window;
	window.show();

	return app.exec();
}
