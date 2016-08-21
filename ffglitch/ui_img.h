// Copyright: Ramiro Polla
// License: GPLv2 or later

#ifndef UI_IMG_H
#define UI_IMG_H

#include <QWidget>

#include "ffglitch.h"
#include "ffimage.h"

class QImage;

class UIImg : public QWidget
{
	Q_OBJECT

public:
	UIImg(QWidget *parent, FFglitch *_ffglitch);
	~UIImg();

	bool update_image();
	bool save_image();

signals:
	void status_updated(const QString &str);

protected:
	void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
	void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

	QSize sizeHint() const Q_DECL_OVERRIDE;

	void update_status(const QPoint &p);

private:
	FFglitch *ffglitch;
	QPoint p_mb;
	QRect r_mb;
	int cur_component;
	int cur_scan;
};

#endif /* UI_IMG_H */
