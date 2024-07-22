#! /usr/bin/bash
video='/tank/home/jli/misc_files/output_test_26_1_s1.ts'
output='/tank/home/jli/misc_files/output_26_1_scte35.txt'
format='-show_frames -only_show_SCTE35 -print_format compact'
parameters='-show_entries frame=PCR,media_type,packet_num,stream_index,key_frame,pkt_pts,pkt_pts_time,pict_type,SCTE35_REMAINING'

./ffprobe $format $parameters $video 2>&1 | tee $output
