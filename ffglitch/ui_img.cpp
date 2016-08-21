// Copyright: Ramiro Polla
// License: GPLv2 or later

#include <QtWidgets>
#include <QFileInfo>

#include "ui_img.h"

#include "ffglitch.h"

UIImg::UIImg(QWidget *parent)
    : QWidget(parent)
{
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	r_mb.setX(-1);
	r_mb.setY(-1);

	cur_component = 0;
	cur_scan = 0;
}

bool UIImg::open_image(const QString &_fname)
{
	bool ok = ffglitch.open_image(_fname.toLatin1());
	if ( ok )
	{
		fname = _fname;
		resize(ffglitch.get_image()->qimage->size());
		update_image();
	}
	return ok;
}

bool UIImg::save_image(const QString &_fname)
{
	return ffglitch.save_image(_fname.toLatin1());
}

QString UIImg::get_fname()
{
	return fname;
}

void UIImg::paintEvent(QPaintEvent *event)
{
	FFImage *image = ffglitch.get_image();
	if ( image == NULL )
		return;

	QPainter painter(this);
	QRect rect = event->rect();
	painter.drawImage(rect, *image->qimage, rect);

	if ( p_mb.x() != -1 && p_mb.y() != -1 && rect.contains(r_mb) )
		painter.fillRect(r_mb, QColor(0, 0, 0, 63));
}

void UIImg::update_status(const QPoint &p)
{
	FFImage *image = ffglitch.get_image();
	if ( image == NULL )
		return;

	QString status;
	struct ffglitch_mjpeg_dc *ctx;
	ctx = image->dc_coeffs;
	ffglitch_mjpeg_dc_set_scan(ctx, cur_scan);
	struct ffglitch_mjpeg_dc_coeffs *dc_ctx = ctx->cur;
	ffglitch_mjpeg_dc_coeffs_set_cur_yx(dc_ctx, cur_component, p.y(), p.x());

	QTextStream qts(&status);
	if ( ctx->nb_scans > 1 )
		qts << "scan[" << cur_scan << "/" << ctx->nb_scans << "] ";
	qts << "dc[" << cur_component << "/" << dc_ctx->nb_components << "] ";
	qts << "[" << p.x() / 8 << "][" << p.y() / 8 << "] = ";
	qts << *dc_ctx->cur;
	emit status_updated(status);
}

void UIImg::mouseMoveEvent(QMouseEvent *event)
{
	FFImage *image = ffglitch.get_image();
	if ( image == NULL )
		return;

	const QPoint &p = event->pos();
	const QPoint new_p_mb(p.x() / 8, p.y() / 8);

	if ( new_p_mb != p_mb )
	{
		struct ffglitch_mjpeg_dc *ctx;
		ctx = image->dc_coeffs;
		ffglitch_mjpeg_dc_set_scan(ctx, cur_scan);
		struct ffglitch_mjpeg_dc_coeffs *dc_ctx = ctx->cur;

		int mb_r_x = new_p_mb.x();
		int mb_r_y = new_p_mb.y();
		int mb_r_w = 8;
		int mb_r_h = 8;
		if ( cur_component == 1 || cur_component == 2 )
		{
			mb_r_x &= ~dc_ctx->chroma_h_shift;
			mb_r_y &= ~dc_ctx->chroma_v_shift;
			mb_r_w <<= dc_ctx->chroma_h_shift;
			mb_r_h <<= dc_ctx->chroma_v_shift;
		}

		QRect new_r_mb(mb_r_x * 8, mb_r_y * 8, mb_r_w, mb_r_h);
		QRect old_r_mb = r_mb;

		r_mb = new_r_mb;
		p_mb = new_p_mb;

		update(old_r_mb);
		update(r_mb);
	}

	update_status(p);
}

bool UIImg::update_image()
{
	if ( !ffglitch.update_image() )
		return false;
	update();
	return true;
}

void UIImg::wheelEvent(QWheelEvent *event)
{
	FFImage *image = ffglitch.get_image();
	if ( image == NULL )
		return;

	const QPoint &p = event->pos();
	const QPoint &d = event->angleDelta();

	struct ffglitch_mjpeg_dc *ctx;
	ctx = image->dc_coeffs;
	ffglitch_mjpeg_dc_set_scan(ctx, cur_scan);
	struct ffglitch_mjpeg_dc_coeffs *dc_ctx = ctx->cur;
	ffglitch_mjpeg_dc_coeffs_set_cur_yx(dc_ctx, cur_component, p.y(), p.x());

	// TODO: make this an option and a function
	int val = *dc_ctx->cur;
	if ( d.y() > 0 )
		val++;
	else
		val--;
	int max = ((cur_component == 1) || (cur_component == 2))
	        ? dc_ctx->chroma_max
	        : dc_ctx->luma_max;
	int min = -max;
	if ( val >= min && val <= max )
	{
		*dc_ctx->cur = val;
		if ( update_image() )
			update_status(p);
	}
}

void UIImg::mousePressEvent(QMouseEvent *event)
{
	FFImage *image = ffglitch.get_image();
	if ( image == NULL )
		return;

	const QPoint &p = event->pos();
	struct ffglitch_mjpeg_dc *ctx;
	ctx = image->dc_coeffs;
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
