# Pixel format glitch art

Pixel format glitch art consists of forcing FFmpeg to perform an incorrect pixel format conversion.

Suppose we have `<input.png>` with whatever pixel format. We first tell FFmpeg to convert that file to an intermediate raw file with a specific pixel format:

    <input.png> -> [yuv420p] <intermediate.raw>

and then we tell FFmpeg to treat the intermediate file as having some other pixel format, convert again to the intermediate pixel format, and save the output:

    [nv12] <intermediate.raw> -> [yuv420p] -> <output.png>

# Using

First, edit the first lines in the script to specify where to find `ffmpeg` and `ffprobe`, the directory to put the output files, and the output file extension.

The script expects an `input` file and creates a `makefile` with all possible combinations of pixel format conversions. You should then run that makefile with `-k`, since some conversions will fail. Use as many cores as you can with `-jN`, where `N` is the number of cores.

    $ python pix_fmts.py <input.png> > makefile
    $ make -r -k -jN

It might be wise to remove all the duplicates afterwards (there will be a whole bunch of duplicates):

    $ cd output_dir
    $ fdupes --delete --noprompt .

# Have fun

You now have a few gigabytes of corrupted images to play with. Have fun!

# Samples

![libcaca-logo](samples/libcaca-logo.png)
![gbrp12le->yuv420p10le](samples/gbrp12le_yuv420p10le.png)
![gbrp9be->bgr48le](samples/gbrp9be_bgr48le.png)
![gbrp9be->yuv422p10le](samples/gbrp9be_yuv422p10le.png)
![gbrp->yuvj440p](samples/gbrp_yuvj440p.png)
![xyz12be->yuv420p10le](samples/xyz12be_yuv420p10le.png)
![yuv444p12le->yuv444p16be](samples/yuv444p12le_yuv444p16be.png)
![yuv444p16be->yuv420p14be](samples/yuv444p16be_yuv420p14be.png)
![yuva422p9be->yuv444p14le](samples/yuva422p9be_yuv444p14le.png)
![yuva422p9le->yuv420p16be](samples/yuva422p9le_yuv420p16be.png)
![yuvj444p->yuv420p12le](samples/yuvj444p_yuv420p12le.png)
