// Copyright: Ramiro Polla
// License: GPLv2 or later

#ifndef UI_MAIN_H
#define UI_MAIN_H

#include <QMainWindow>

#include "ui_img.h"

class FFImage;
class QMdiArea;

class UIMain : public QMainWindow
{
	Q_OBJECT

public:
	UIMain();

	bool open_file(const QString &fname);
	bool save_file(const QString &fname);

private slots:
	void open();
	void save_as();
	void exit();
	void about();

	void update_status(const QString &str);

private:
	void createMenu();

	QMdiArea *mdi_area;

	QMenu *file_menu;
	QAction *file_open_act;
	QAction *file_save_as_act;
	QAction *exit_act;

	QMenu *help_menu;
	QAction *help_about_act;

	UIImg *current_image() const;
};

#endif /* UI_MAIN_H */
