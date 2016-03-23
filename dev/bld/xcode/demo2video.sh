#!/bin/sh
./glfwplayer -w -r dummy demo_remix.xml
# ffmpeg -y -i output.mp4 -i Phenomena-Remix.mp3 -codec copy -vf scale=640:360 DemoRemixed.avi
ffmpeg -y -i output.mp4 -i Phenomena-Remix.mp3 -acodec copy -vcodec h264 -vf scale=640:360 DemoRemixed.avi


