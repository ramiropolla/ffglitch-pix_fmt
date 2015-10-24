
# edit these variables for your needs
ffmpeg = "ffmpeg -hide_banner -v error"
ffprobe = "ffprobe"
output_dir = "output_dir"
output_ext = "png"
shutup = " > /dev/null"

# imports
import subprocess
import json
import sys
import re

# check arguments
if len(sys.argv) != 2:
    sys.stderr.write("usage: %s <input image>\n" % sys.argv[0])
    sys.exit(1)
fname = sys.argv[1]

# Get input file dimensions with FFprobe
cmd = ffprobe + " -v error -print_format json -show_streams " + fname
process = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
(stdout, stderr) = process.communicate()
exit_code = process.wait()

if exit_code != 0:
    sys.stderr.write("Could not run FFprobe to get input dimensions\n")
    sys.exit(exit_code)

ffprobe_output = json.loads(stdout)
img_w = ffprobe_output['streams'][0]['width']
img_h = ffprobe_output['streams'][0]['height']
dimension = "%dx%d" % (img_w, img_h)

# Get list of pixel formats from FFmpeg
cmd = ffmpeg + " -pix_fmts"
process = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
(stdout, stderr) = process.communicate()
exit_code = process.wait()

if exit_code != 0:
    sys.stderr.write("Could not run FFmpeg to get list of pixel formats\n")
    sys.exit(exit_code)

# Generate pixel format combinations
pix_fmts_in = []
pix_fmts_out = []

pix_fmt_matcher = re.compile("(.....) (\w+)")

started = False
for line in stdout.split('\n'):
    if "-----" in line:
        started = True
        continue
    if not started or not line:
        continue
    match = pix_fmt_matcher.match(line)
    flags = match.group(1)
    pix_fmt = match.group(2)
    if 'I' in flags:
        pix_fmts_in.append(pix_fmt)
    if 'O' in flags:
        pix_fmts_out.append(pix_fmt)

combinations = {}
for pix_fmt_in in pix_fmts_in:
    if pix_fmt_in not in pix_fmts_out:
        continue
    combinations[pix_fmt_in] = []
    for pix_fmt_out in pix_fmts_out:
        combinations[pix_fmt_in].append(pix_fmt_out)

# Write variables
print "FFMPEG = %s" % ffmpeg
print "Q = @"

# Write all
print "all: \\"
for pix_fmt_in, pix_fmt_list in combinations.iteritems():
    for pix_fmt_out in pix_fmt_list:
        out_file = output_dir + "/" + pix_fmt_out + "_" + pix_fmt_in + "." + output_ext
        print "\t%s\\" % "{0:<72}".format(out_file)
print ""

# Write output requisites
for pix_fmt_in, pix_fmt_list in combinations.iteritems():
    for pix_fmt_out in pix_fmt_list:
        out_file = output_dir + "/" + pix_fmt_out + "_" + pix_fmt_in + "." + output_ext
        in_file = output_dir + "/" + pix_fmt_in + ".raw"
        print out_file + ": " + in_file

# Write output rules
for pix_fmt_in, pix_fmt_list in combinations.iteritems():
    if pix_fmt_list:
        print "%%_%s.%s: %%.raw" % (pix_fmt_in, output_ext)
        print "\t$(Q)echo \"[%s] -> [$(subst _%s,,$(basename $(@F)))]\"" % (pix_fmt_in, pix_fmt_in)
        print "\t$(Q)$(FFMPEG) -f rawvideo -pix_fmt " + pix_fmt_in + " -s " + dimension + " -i $< -vframes 1 -y $@" + shutup

# Write intermediate rule
print "%%.raw: %s | %s" % (fname, output_dir)
print "\t$(Q)echo intermediate [$(basename $(@F))]"
print "\t$(Q)$(FFMPEG) -i $< -f rawvideo -pix_fmt $(basename $(@F)) -vframes 1 -y $@" + shutup

# Write mkdir rule
print "%s:" % output_dir
print "\t$(Q)mkdir -p $@"

# Write clean rule
print "clean::"
print "\trm -rf %s" % output_dir
