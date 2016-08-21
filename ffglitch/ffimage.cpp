// Copyright: Ramiro Polla
// License: GPLv2 or later

#include <QImage>

#include "ffimage.h"

FFImage::FFImage(AVFrame *iframe)
{
	dc_coeffs = NULL;

	oframe = av_frame_alloc();
	oframe->format = AV_PIX_FMT_ARGB;
	oframe->width  = iframe->width;
	oframe->height = iframe->height;
	av_frame_get_buffer(oframe, 32);

	qimage = new QImage(oframe->data[0], oframe->width, oframe->height, oframe->linesize[0], QImage::Format_ARGB32);
}

FFImage::~FFImage()
{
	if ( qimage != NULL )
		delete qimage;
	if ( oframe != NULL )
		av_frame_free(&oframe);
	if ( dc_coeffs != NULL )
		ffglitch_mjpeg_dc_freep(&dc_coeffs);
}
