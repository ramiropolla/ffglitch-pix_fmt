// Copyright: Ramiro Polla
// License: GPLv2 or later

#ifndef FFGLITCH_H
#define FFGLITCH_H

class FFImage;

typedef struct AVCodecContext AVCodecContext;
typedef struct AVPacket AVPacket;
typedef struct AVFrame AVFrame;
typedef struct SwsContext SwsContext;

struct FFglitch
{
	FFglitch();
	~FFglitch();

	bool open_image(const char *_fname);
	bool update_image();
	bool save_image(const char *_fname);

	void reset();
	FFImage *get_image();
	const char *get_fname();

private:
	bool decode_ipkt(AVFrame *iframe, bool import, bool apply);
	bool out_iframe(AVFrame *oframe, AVFrame *iframe);

	AVCodecContext *ctx_idec;
	AVPacket *ipkt;
	SwsContext *ctx_sws;

	const char *fname;
	FFImage *image;
};

#endif /* FFGLITCH_H */
