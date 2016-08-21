// Copyright: Ramiro Polla
// License: GPLv2 or later

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "ui_main.h"
#include "ui_img.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	QCoreApplication::setApplicationName("FFglitch");
	QCoreApplication::setOrganizationName("arrozcru");
	QCoreApplication::setApplicationVersion("0.0.003");
	QCommandLineParser parser;
	parser.setApplicationDescription("FFglitch");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("file", "The file to open.");
	parser.process(app);

	UIMain ui_main;
	foreach (const QString &fname, parser.positionalArguments())
		ui_main.open_file(fname);
	ui_main.setWindowState(Qt::WindowMaximized);
	ui_main.show();

	return app.exec();
}
