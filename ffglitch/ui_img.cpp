// Copyright: Ramiro Polla
// License: GPLv2 or later

#include <QtWidgets>
#include <QFileInfo>

#include "ui_img.h"

#include "ffglitch.h"

UIImg::UIImg(QWidget *parent, FFglitch *_ffglitch)
    : QWidget(parent)
{
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	r_mb.setX(-1);
	r_mb.setY(-1);
	r_mb.setWidth(10);
	r_mb.setHeight(10);

	cur_component = 0;
	cur_scan = 0;

	ffglitch = _ffglitch;
}

UIImg::~UIImg()
{
	delete ffglitch;
}

bool UIImg::save_image()
{
	QString fname(ffglitch->get_fname());

	QFileInfo finfo(fname);
	finfo.setFile(QString("ffglitch_") + finfo.fileName());

	fname = finfo.absoluteFilePath();
	fname = QFileDialog::getSaveFileName(this, "Save As", fname);

	if ( fname.isEmpty() )
		return false;

	return ffglitch->save_image(fname.toLatin1());
}

void UIImg::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	QRect rect = event->rect();
	painter.drawImage(rect, *ffglitch->get_image()->qimage, rect);

	if ( p_mb.x() != -1 && p_mb.y() != -1 && rect.contains(r_mb) )
		painter.fillRect(r_mb, QColor(0, 0, 0, 63));
}

void UIImg::update_status(const QPoint &p)
{
	QString status;
	struct ffglitch_mjpeg_dc *ctx;
	ctx = ffglitch->get_image()->dc_coeffs;
	ffglitch_mjpeg_dc_set_scan(ctx, cur_scan);
	struct ffglitch_mjpeg_dc_coeffs *dc_ctx = ctx->cur;
	ffglitch_mjpeg_dc_coeffs_set_cur_yx(dc_ctx, cur_component, p.y(), p.x());

	QTextStream qts(&status);
	if ( ctx->nb_scans > 1 )
		qts << "scan[" << cur_scan << "/" << ctx->nb_scans << "] ";
	qts << "dc[" << cur_component << " / " << dc_ctx->nb_components << "] ";
	qts << "[" << p.x() / 8 << "][" << p.y() / 8 << "] = ";
	qts << *dc_ctx->cur;
	emit status_updated(status);
}

void UIImg::mouseMoveEvent(QMouseEvent *event)
{
	const QPoint &p = event->pos();
	const QPoint new_p_mb(p.x() / 8, p.y() / 8);

	if ( new_p_mb != p_mb )
	{
		QRect new_r_mb(new_p_mb.x() * 8, new_p_mb.y() * 8, 8, 8);
		QRect old_r_mb = r_mb;

		r_mb = new_r_mb;
		p_mb = new_p_mb;

		update(old_r_mb);
		update(r_mb);
	}

	update_status(p);
}

void UIImg::wheelEvent(QWheelEvent *event)
{
	const QPoint &p = event->pos();
	const QPoint &d = event->angleDelta();

	struct ffglitch_mjpeg_dc *ctx;
	ctx = ffglitch->get_image()->dc_coeffs;
	ffglitch_mjpeg_dc_set_scan(ctx, cur_scan);
	struct ffglitch_mjpeg_dc_coeffs *dc_ctx = ctx->cur;
	ffglitch_mjpeg_dc_coeffs_set_cur_yx(dc_ctx, cur_component, p.y(), p.x());

	if ( d.y() > 0 )
		(*dc_ctx->cur)++;
	else
		(*dc_ctx->cur)--;

	if ( ffglitch->update_image() )
	{
		update();
		update_status(p);
	}
}

void UIImg::mousePressEvent(QMouseEvent *event)
{
	const QPoint &p = event->pos();
	struct ffglitch_mjpeg_dc *ctx;
	ctx = ffglitch->get_image()->dc_coeffs;
	if ( event->button() == Qt::RightButton )
	{
		cur_scan++;
		if ( cur_scan == ctx->nb_scans )
			cur_scan = 0;
	}
	ffglitch_mjpeg_dc_set_scan(ctx, cur_scan);
	if ( event->button() == Qt::LeftButton )
	{
		struct ffglitch_mjpeg_dc_coeffs *dc_ctx = ctx->cur;
		cur_component++;
		if ( cur_component == dc_ctx->nb_components )
			cur_component = 0;
	}
	update_status(p);
}

void UIImg::leaveEvent(QEvent * /* event */)
{
	p_mb = QPoint(-1, -1);
	update(r_mb);

	QString status;
	emit status_updated(status);
}

QSize UIImg::sizeHint() const
{
	return ffglitch->get_image()->qimage->size();
}
