// Copyright: Ramiro Polla
// License: GPLv2 or later

#ifndef PY_FFGLITCH_H
#define PY_FFGLITCH_H

#include <vector>
#include <string>

extern "C" {
#include <libavcodec/ffglitch.h>
}

struct PyFFglitch
{
	PyFFglitch();
	~PyFFglitch();

	int run_script(const char *fname);

	int add_dc_coeffs(const char *name, struct ffglitch_mjpeg_dc *ctx);
	int del_dc_coeffs(const char *name);

	struct mjpeg_dc_item
	{
		std::string name;
		struct ffglitch_mjpeg_dc *ctx;
	};
	std::vector<mjpeg_dc_item> mjpeg_dc;
};

extern PyFFglitch py_ffglitch;

#endif /* PY_FFGLITCH_H */
