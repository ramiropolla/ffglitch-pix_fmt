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
	const FFImage *get_image() const;
	const char *get_fname();

#define DEC_IMPORT 1
#define DEC_EXPORT 2
#define DEC_APPLY  4

private:
	bool decode_ipkt(AVFrame *iframe, int flags);
	bool out_iframe(AVFrame *oframe, AVFrame *iframe);

	AVCodecContext *ctx_idec;
	AVPacket *ipkt;
	SwsContext *ctx_sws;

	const char *fname;
	FFImage *image;
};

#endif /* FFGLITCH_H */
