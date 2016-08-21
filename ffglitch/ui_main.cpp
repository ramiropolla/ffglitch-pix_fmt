// Copyright: Ramiro Polla
// License: GPLv2 or later

#include <QApplication>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollArea>

#include "ui_main.h"

#include "ffimage.h"
#include "py_ffglitch.h"

UIMain::UIMain()
    : scroll_area(new QScrollArea)
    , ui_img(new UIImg(this))
{
	setWindowTitle("FFglitch");

	scroll_area->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	scroll_area->setWidget(ui_img);
	scroll_area->setVisible(false);
	setCentralWidget(scroll_area);

	connect(ui_img, &UIImg::status_updated, this, &UIMain::update_status);

	statusBar()->clearMessage();

	createMenu();
}

void UIMain::createMenu()
{
	file_menu = menuBar()->addMenu(tr("&File"));

	file_open_act = new QAction(tr("&Open..."), this);
	file_open_act->setShortcuts(QKeySequence::Open);
	connect(file_open_act, &QAction::triggered, this, &UIMain::open);
	file_menu->addAction(file_open_act);

	file_run_script_act = new QAction(tr("&Run Script..."), this);
	file_run_script_act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F2));
	connect(file_run_script_act, &QAction::triggered, this, &UIMain::run_script);
	file_menu->addAction(file_run_script_act);

	file_save_as_act = new QAction(tr("Save &As..."), this);
	file_save_as_act->setShortcuts(QKeySequence::SaveAs);
	connect(file_save_as_act, &QAction::triggered, this, &UIMain::save_as);
	file_menu->addAction(file_save_as_act);

	file_menu->addSeparator();

	exit_act = new QAction(tr("E&xit"), this);
	exit_act->setShortcuts(QKeySequence::Quit);
	connect(exit_act, &QAction::triggered, this, &UIMain::exit);
	file_menu->addAction(exit_act);

	menuBar()->addSeparator();

	help_menu = menuBar()->addMenu(tr("&Help"));

	help_about_act = new QAction(tr("&About"), this);
	connect(help_about_act, &QAction::triggered, this, &UIMain::about);
	help_menu->addAction(help_about_act);
}

void UIMain::open()
{
	const QString fname = QFileDialog::getOpenFileName(this);

	if ( fname.isEmpty() )
		return;

	open_file(fname);
}

bool UIMain::open_file(const QString &fname)
{
	if ( !ui_img->open_image(fname.toLatin1()) )
	{
		statusBar()->showMessage("Open failed\n");
		return false;
	}
	statusBar()->showMessage("File opened\n");
	scroll_area->setVisible(true);
	ui_img->adjustSize();

	return true;
}

void UIMain::run_script()
{
	const QString fname = QFileDialog::getOpenFileName(this);

	if ( fname.isEmpty() )
		return;

	if ( !run_script_file(fname) )
		statusBar()->showMessage("Script failed\n");
	else
		ui_img->update_image();
}

bool UIMain::run_script_file(const QString &fname)
{
	py_ffglitch.run_script(fname.toLatin1());
	return true;
}

void UIMain::save_as()
{
	QString fname(ui_img->get_fname());

	QFileInfo finfo(fname);
	finfo.setFile(QString("ffglitch_") + finfo.fileName());

	fname = finfo.absoluteFilePath();
	fname = QFileDialog::getSaveFileName(this, "Save As", fname);

	if ( fname.isEmpty() )
		return;

	if ( !ui_img->save_image(fname) )
		statusBar()->showMessage("Save failed\n");
	else
		statusBar()->showMessage("File saved\n");
}

void UIMain::exit()
{
	qApp->closeAllWindows();
}

void UIMain::update_status(const QString &str)
{
	statusBar()->showMessage(str);
}

void UIMain::about()
{
	// TODO: this about box is shitty
	QMessageBox::about(this, tr("About FFglitch"),
	                   "You want to help? Thanks!<br />"
	                   "<br />"
	                   "Go here: <a href='http://ffglitch.arrozcru.org'>FFglitch</a><br />"
	                   "<br />"
	                   "License: GPLv2 or later<br />"
	                   "<br />"
	                   "- Ramiro Polla");
}
