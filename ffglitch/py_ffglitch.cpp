// Copyright: Ramiro Polla
// License: GPLv2 or later

#include "py_ffglitch.h"

extern "C" {
#include <Python.h>
}

#include <libavcodec/ffglitch.h>

PyFFglitch py_ffglitch;

static PyObject *
py_ffglitch_from_mjpeg_dc(struct ffglitch_mjpeg_dc *ctx)
{
	PyObject *py_scans;
	int scan;

	ffglitch_mjpeg_dc_reset_scan(ctx);

	py_scans = PyList_New(ctx->nb_scans);
	for ( scan = 0; scan < ctx->nb_scans; scan++ )
	{
		struct ffglitch_mjpeg_dc_coeffs *coeffs;
		PyObject *py_mbs;
		int mb_y;

		ffglitch_mjpeg_dc_next_scan(ctx);
		coeffs = ctx->cur;

		py_mbs = PyList_New(coeffs->mb_height);
		for ( mb_y = 0; mb_y < coeffs->mb_height; mb_y++ )
		{
			PyObject *py_v;
			int mb_x;

			py_v = PyList_New(coeffs->mb_width);
			for ( mb_x = 0; mb_x < coeffs->mb_width; mb_x++ )
			{
				PyObject *py_h;
				int component;

				py_h = PyList_New(coeffs->nb_components);
				for ( component = 0; component < coeffs->nb_components; component++ )
				{
					PyObject *py_blocks;
					int block;

					py_blocks = PyList_New(coeffs->nb_blocks[component]);
					for ( block = 0; block < coeffs->nb_blocks[component]; block++ )
					{
						PyObject *py_b;
						ffglitch_mjpeg_dc_coeffs_set_cur(coeffs, component, mb_y, mb_x, block);
						py_b = PyInt_FromLong(*coeffs->cur);
						PyList_SET_ITEM(py_blocks, block, py_b);
					}
					PyList_SET_ITEM(py_h, component, py_blocks);
				}
				PyList_SET_ITEM(py_v, mb_x, py_h);
			}
			PyList_SET_ITEM(py_mbs, mb_y, py_v);
		}
		PyList_SET_ITEM(py_scans, scan, py_mbs);
	}

	return py_scans;
}

static PyObject *
py_ffglitch_get_dc_coeffs(PyObject *, PyObject *args)
{
	Py_ssize_t nargs = PyTuple_Size(args);

	if ( nargs != 0 )
		Py_RETURN_NONE;

	return py_ffglitch_from_mjpeg_dc(py_ffglitch.mjpeg_dc[0].ctx);
}

static bool
py_ffglitch_to_mjpeg_dc(struct ffglitch_mjpeg_dc *ctx, PyObject *pyobj)
{
	PyObject *py_scans;
	int scan;

	ffglitch_mjpeg_dc_reset_scan(ctx);

	py_scans = pyobj;
	if ( PyList_Size(py_scans) != ctx->nb_scans )
		return false;
	for ( scan = 0; scan < ctx->nb_scans; scan++ )
	{
		struct ffglitch_mjpeg_dc_coeffs *coeffs;
		PyObject *py_mbs;
		int mb_y;

		ffglitch_mjpeg_dc_next_scan(ctx);
		coeffs = ctx->cur;

		py_mbs = PyList_GetItem(py_scans, scan);
		if ( PyList_Size(py_mbs) != coeffs->mb_height )
			return false;
		for ( mb_y = 0; mb_y < coeffs->mb_height; mb_y++ )
		{
			PyObject *py_v;
			int mb_x;

			py_v = PyList_GetItem(py_mbs, mb_y);
			if ( PyList_Size(py_v) != coeffs->mb_width )
				return false;
			for ( mb_x = 0; mb_x < coeffs->mb_width; mb_x++ )
			{
				PyObject *py_h;
				int component;

				py_h = PyList_GetItem(py_v, mb_x);
				if ( PyList_Size(py_h) != coeffs->nb_components )
					return false;
				for ( component = 0; component < coeffs->nb_components; component++ )
				{
					PyObject *py_blocks;
					int block;

					py_blocks = PyList_GetItem(py_h, component);
					if ( PyList_Size(py_blocks) != coeffs->nb_blocks[component] )
						return false;
					for ( block = 0; block < coeffs->nb_blocks[component]; block++ )
					{
						PyObject *py_b;
						ffglitch_mjpeg_dc_coeffs_set_cur(coeffs, component, mb_y, mb_x, block);
						py_b = PyList_GetItem(py_blocks, block);
						*coeffs->cur = PyInt_AsLong(py_b);
					}
				}
			}
		}
	}

	return true;
}

static PyObject *
py_ffglitch_set_dc_coeffs(PyObject *, PyObject *args)
{
	PyObject *py_scans;

	if ( !PyArg_ParseTuple(args, "O:set_dc_coeffs", &py_scans) )
		Py_RETURN_FALSE;

	if ( !py_ffglitch_to_mjpeg_dc(py_ffglitch.mjpeg_dc[0].ctx, py_scans) )
		Py_RETURN_FALSE;

	Py_RETURN_TRUE;
}

static PyMethodDef py_ffglitch_methods[] =
{
	{ "get_dc_coeffs", py_ffglitch_get_dc_coeffs, METH_VARARGS, NULL },
	{ "set_dc_coeffs", py_ffglitch_set_dc_coeffs, METH_VARARGS, NULL },
	{ NULL, NULL, 0, NULL }
};

PyFFglitch::PyFFglitch()
{

	Py_SetProgramName("ffglitch");
	Py_Initialize();

	Py_InitModule("ffglitch", py_ffglitch_methods);
}

int PyFFglitch::run_script(const char *fname)
{
	FILE *fp = fopen(fname, "rt");
	if ( fp == NULL )
		return -1;

	PyRun_SimpleFile(fp, fname);
	fclose(fp);

	return 0;
}

int PyFFglitch::add_dc_coeffs(const char *name, struct ffglitch_mjpeg_dc *ctx)
{
	mjpeg_dc_item item = { name, ctx };
	mjpeg_dc.push_back(item);
	return 0;
}

int PyFFglitch::del_dc_coeffs(const char *name)
{
	std::vector<mjpeg_dc_item>::iterator it;
	for ( it = mjpeg_dc.begin(); it != mjpeg_dc.end(); )
		if ( it->name == name )
			it = mjpeg_dc.erase(it);
		else
			++it;
	return 0;
}

PyFFglitch::~PyFFglitch()
{
	Py_Finalize();
}
