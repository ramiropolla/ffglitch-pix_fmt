// Copyright: Ramiro Polla
// License: GPLv2 or later

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
}

#include "ffglitch.h"
#include "py_ffglitch.h"

#include "ffimage.h"

FFglitch::FFglitch()
{
	image = NULL;
	ctx_idec = NULL;
	ipkt = NULL;
	ctx_sws = NULL;
	fname = NULL;
}

bool FFglitch::decode_ipkt(AVFrame *iframe, bool import, bool apply)
{
	bool ok = false;

	AVDictionary *opts = NULL;
	int ret;

	if ( import )
	{
		int size;
		uint8_t *data;

		data = av_packet_get_side_data(ipkt, FFGLITCH_PKT_DATA_DC_COEFFICIENTS, &size);
		if ( data == NULL )
		{
			size = image->dc_coeffs->size;
			data = av_packet_new_side_data(ipkt, FFGLITCH_PKT_DATA_DC_COEFFICIENTS, size);
			if ( data == NULL )
				goto the_end;
		}
		ffglitch_mjpeg_dc_reset_scan(image->dc_coeffs);
		memcpy(data, image->dc_coeffs, size);

		av_dict_set(&opts, "ffglitch_flags", "-e_dc-a_dc+i_dc", 0);
		if ( apply )
			av_dict_set(&opts, "ffglitch_flags", "-e_dc+a_dc+i_dc", 0);
	}
	else
	{
		av_dict_set(&opts, "ffglitch_flags", "+e_dc-a_dc-i_dc", 0);
	}

	ret = av_opt_set_dict(ctx_idec, &opts);
	if ( ret < 0 )
	{
		fprintf(stderr, "av_opt_set_dict() failed\n");
		goto the_end;
	}

	ret = avcodec_send_packet(ctx_idec, ipkt);
	if ( ret < 0 )
	{
		fprintf(stderr, "avcodec_send_packet() failed\n");
		goto the_end;
	}
	ret = avcodec_receive_frame(ctx_idec, iframe);
	if ( ret < 0 )
	{
		fprintf(stderr, "avcodec_receive_frame() failed\n");
		goto the_end;
	}

	ok = true;

the_end:
	av_dict_free(&opts);

	return ok;
}

bool FFglitch::out_iframe(AVFrame *oframe, AVFrame *iframe)
{
	int ret;

	ret = sws_scale(ctx_sws, iframe->data, iframe->linesize, 0, iframe->height, oframe->data, oframe->linesize);
	if ( ret < 0 )
	{
		fprintf(stderr, "sws_scale() failed\n");
		return false;
	}

	return true;
}

bool FFglitch::open_image(const char *_fname)
{
	bool ok = false;

	AVFormatContext *ctx_ifmt = NULL;
	AVCodec *decoder = NULL;
	AVFrame *iframe = NULL;
	AVFrameSideData *sd = NULL;

	const struct ffglitch_mjpeg_dc *dc_coeffs;

	fname = strdup(_fname);

	int ret;
	ret = avformat_open_input(&ctx_ifmt, fname, NULL, NULL);
	if ( ret < 0 )
	{
		fprintf(stderr, "Could not open input file '%s'\n", fname);
		goto the_end;
	}

	ret = avformat_find_stream_info(ctx_ifmt, 0);
	if ( ret < 0 )
	{
		fprintf(stderr, "Failed to retrieve input stream information\n");
		goto the_end;
	}

	decoder = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
	if ( decoder == NULL )
	{
		fprintf(stderr, "Failed to find MJPEG codec\n");
		goto the_end;
	}

	ctx_idec = avcodec_alloc_context3(decoder);
	ret = avcodec_parameters_to_context(ctx_idec, ctx_ifmt->streams[0]->codecpar);
	if ( ret < 0 )
	{
		fprintf(stderr, "Oopsie?\n");
		goto the_end;
	}

	ret = avcodec_open2(ctx_idec, decoder, NULL);
	if ( ret < 0 )
	{
		fprintf(stderr, "Failed to open decoder\n");
		goto the_end;
	}

	ipkt = av_packet_alloc();

	ret = av_read_frame(ctx_ifmt, ipkt);
	if ( ret < 0 )
	{
		fprintf(stderr, "Failed to read frame\n");
		goto the_end;
	}

	iframe = av_frame_alloc();

	if ( !decode_ipkt(iframe, false, false) )
		goto the_end;

	ctx_sws = sws_getContext(iframe->width, iframe->height, AVPixelFormat(iframe->format),
	                         iframe->width, iframe->height, AV_PIX_FMT_BGRA,
	                         SWS_BILINEAR, NULL, NULL, NULL);
	if ( ctx_sws == NULL )
	{
		fprintf(stderr, "Failed to sws_getContext\n");
		goto the_end;
	}

	image = new FFImage(iframe);

	if ( !out_iframe(image->oframe, iframe) )
		goto the_end;

	sd = av_frame_get_side_data(iframe, FFGLITCH_FRAME_DATA_DC_COEFFICIENTS);
	if ( sd == NULL )
	{
		fprintf(stderr, "Failed to get dc coeffs (?)\n");
		goto the_end;
	}
	dc_coeffs = (const struct ffglitch_mjpeg_dc *) sd->data;
	image->dc_coeffs = ffglitch_mjpeg_dc_copy(dc_coeffs);
	image->dc_coeffs->cur = NULL;

	py_ffglitch.add_dc_coeffs(fname, image->dc_coeffs);

	ok = true;

the_end:
	if ( ctx_ifmt != NULL )
		avformat_close_input(&ctx_ifmt);
	if ( iframe != NULL )
		av_frame_free(&iframe);

	if ( !ok )
		reset();

	return ok;
}

bool FFglitch::update_image()
{
	bool ok = false;

	AVFrame *iframe = NULL;

	iframe = av_frame_alloc();

	if ( !decode_ipkt(iframe, true, false) )
		goto the_end;

	if ( !out_iframe(image->oframe, iframe) )
		goto the_end;

	ok = true;

the_end:
	if ( iframe != NULL )
		av_frame_free(&iframe);

	return ok;
}

bool FFglitch::save_image(const char *_fname)
{
	bool ok = false;

	AVFrame *iframe = NULL;
	AVFrameSideData *sd = NULL;

	FILE *fp;

	iframe = av_frame_alloc();

	if ( !decode_ipkt(iframe, true, true) )
		goto the_end;

	sd = av_frame_get_side_data(iframe, FFGLITCH_FRAME_DATA_OUT_IMAGE);
	if ( sd == NULL )
	{
		fprintf(stderr, "Failed to get output image\n");
		goto the_end;
	}

	fp = fopen(_fname, "wb");
	if ( fp == NULL )
		goto the_end;
	fwrite((const char *) sd->data, sd->size, 1, fp);
	fclose(fp);

	ok = true;

the_end:
	if ( iframe != NULL )
		av_frame_free(&iframe);

	return ok;
}

FFImage *FFglitch::get_image()
{
	return image;
}

const char *FFglitch::get_fname()
{
	return fname;
}

void FFglitch::reset()
{
	if ( ipkt != NULL )
		av_packet_free(&ipkt);
	if ( ctx_idec != NULL )
		avcodec_free_context(&ctx_idec);
	if ( ctx_sws != NULL )
	{
		sws_freeContext(ctx_sws);
		ctx_sws = NULL;
	}
	if ( image != NULL )
	{
		delete image;
		image = NULL;
	}
	if ( fname != NULL )
		py_ffglitch.del_dc_coeffs(fname);
	free((void *) fname);
	fname = NULL;
}

FFglitch::~FFglitch()
{
	reset();
}
