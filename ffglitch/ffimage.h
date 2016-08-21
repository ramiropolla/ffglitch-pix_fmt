// Copyright: Ramiro Polla
// License: GPLv2 or later

#ifndef FFIMAGE_H
#define FFIMAGE_H

extern "C" {
#include <libavcodec/ffglitch.h>
#include <libavutil/frame.h>
}

class QImage;

class FFImage
{
public:
	FFImage(AVFrame *iframe);
	~FFImage();
	struct ffglitch_mjpeg_dc *dc_coeffs;
	AVFrame *oframe;
	QImage *qimage;
};

#endif /* FFIMAGE_H */
