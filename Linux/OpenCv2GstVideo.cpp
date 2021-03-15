#include <gst/gst.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <chrono>
//#include "Header.h"
#include <opencv2/opencv.hpp>

int Width;
int Height;
int Framerate;
cv::VideoCapture cap;

gboolean LinkElementsManually(GstElement *stream, GstElement *muxer)
{
	gchar *req_pad_name;
	GstPad *req_pad;
	GstPad *static_pad;

	/* Get source pad from queue pipeline */
	static_pad = gst_element_get_static_pad(stream, "src");
	/* Get sink pad from muxer */
	req_pad = gst_element_request_pad(muxer, gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(muxer), "sink_%d"), NULL, NULL);

	req_pad_name = gst_pad_get_name(req_pad);

	g_print("stream Pad name for Muxer %s\n", req_pad_name);
	g_free(req_pad_name);
	/* Link Both src-> sink pads */
	if (GST_IS_PAD(static_pad) && GST_IS_PAD(req_pad))
	{
		int ret = gst_pad_link(static_pad, req_pad);
		if (ret == GST_PAD_LINK_OK)
			return 1; //success
		else
			g_print("Error %s\n", ret);
		//return 0; // failure
	}
	else
		return 0; // failure
}

static void need_data_cv_image_data(GstElement *appsrc, guint unused_size, gpointer    user_data)
{
	static GstClockTime timestamp = 0;
	GstBuffer *buffer;
	guint size, depth, height, width, channels;
	GstFlowReturn ret;
	cv::Mat img;
	guchar *cv_imgData;
	GstMapInfo map;

	cap.read(img);

	//= cv::imread("sample_png.png", CV_LOAD_IMAGE_COLOR);
	//cv::resize(img, img, cv::Size(320, 240));
	height = img.rows;	 width = img.cols;
	channels = (guint)img.channels();
	cv_imgData = (guchar *)img.data;
	size = height*width*channels;

	/* Allocate opencv image size buffer */
	buffer = gst_buffer_new_allocate(NULL, size, NULL);
	gst_buffer_map(buffer, &map, GST_MAP_WRITE);
	memcpy((guchar *)map.data, img.data, size);

	/* Addition of time stamp to buffer */
	GST_BUFFER_PTS(buffer) = timestamp;
	GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, Framerate);
	timestamp += GST_BUFFER_DURATION(buffer);

	/* Emit buffer with signal */
	g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);

	if (ret != GST_FLOW_OK) {
		/* something wrong, stop pushing */
		//g_main_loop_quit(loop);
		gst_buffer_unmap(buffer, &map);
		gst_buffer_unref(buffer);
		g_print("Failed to push Video Buffer\n");
	}

	gst_buffer_unmap(buffer, &map);
	gst_buffer_unref(buffer);
}

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
	GMainLoop *loop = (GMainLoop *)data;

	switch (GST_MESSAGE_TYPE(msg)) {
		gchar  *debug;
		GError *error;

	case GST_MESSAGE_EOS:
		g_print("End of stream\n");
		g_main_loop_quit(loop);
		break;

	case GST_MESSAGE_ERROR:

		gst_message_parse_error(msg, &error, &debug);
		g_free(debug);

		g_printerr("Error: %s\n", error->message);
		g_printerr("Debug Information: %s\n", debug);
		g_error_free(error);

		g_main_loop_quit(loop);
		break;
	default:
		break;
	}

	return TRUE;
}

void Open_Cv_Capture()
{
	cv::Mat frame;
	cap.open(0);

	if (!cap.isOpened())
	{
		printf("Unable to open Webcam!\n");
	}// read cv frame to get width and height
	cap.read(frame);
	/* Required to create input pipeline */
	Width = frame.cols;
	Height = frame.rows;
	Framerate = 25;
}

/* Main function begins from here */
int main(int argc, char *argv[])
{
	/* Initialization */
	gst_init(&argc, &argv);

	GMainLoop *loop;	GstBus *bus;
	guint bus_watch_id;
	GstElement *udpsink;

	loop = g_main_loop_new(NULL, FALSE);
	/* Initialize Video capture from Opencv */
	Open_Cv_Capture();

	/* Create gstreamer elements */
	GstElement *pipeline = gst_pipeline_new("OpenCv2GstPipeline");
	GstElement *cv_source = gst_element_factory_make("appsrc", "cv_image_source");
	GstElement *v_videorate = gst_element_factory_make("videorate", "v_videorate");
	GstElement *v_convert = gst_element_factory_make("videoconvert", "videoconvert");
	GstElement *v_rawCapsfilter = gst_element_factory_make("capsfilter", "raw_video_filer");
	GstElement *v_encCapsfilter = gst_element_factory_make("capsfilter", "encoder_filter");
	GstElement *encoder = gst_element_factory_make("x264enc", "encoder");
	GstElement *mux = gst_element_factory_make("mpegtsmux", "muxer");
	GstElement *v_queue = gst_element_factory_make("queue", "video-queue");

	GstElement *filesink = gst_element_factory_make("filesink", "file-output");


	if (!pipeline || !cv_source || !encoder || !mux || !v_convert || !filesink || !v_encCapsfilter || !v_rawCapsfilter) {
		g_printerr("One element could not be created. Exiting.\n");
		return -1;
	}

	/* Set input video file for source element */
	g_object_set(G_OBJECT(filesink), "location", "OutputFile.ts", NULL);

	/* we add a message handler */
	bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
	bus_watch_id = gst_bus_add_watch(bus, bus_call, loop);
	gst_object_unref(bus);

	/* Add all elements into the pipeline */
	gst_bin_add_many(GST_BIN(pipeline), cv_source, v_videorate, v_convert, v_rawCapsfilter, encoder, v_encCapsfilter, v_queue, mux, filesink, NULL); //fpssink

	/* Do Timestamp on buffer */																															  /* Set Appsrc to 'do-timestamp' property which adds automatic timestamp on received data */																																																																	  /* this instructs appsrc that we will be dealing with timed buffer */
	gst_util_set_object_arg(G_OBJECT(cv_source), "format", "time");

	/* setup */
	g_object_set(G_OBJECT(cv_source), "caps",
		gst_caps_new_simple("video/x-raw",
			"format", G_TYPE_STRING, "BGR",	/* Input Opencv Image format is BGR */
			"width", G_TYPE_INT, Width,
			"height", G_TYPE_INT, Height,
			"framerate", GST_TYPE_FRACTION, Framerate, 1,
			"parsed", G_TYPE_BOOLEAN, TRUE, "sparse", G_TYPE_BOOLEAN, TRUE,
			NULL), NULL);

	/* setup appsrc */
	g_object_set(G_OBJECT(cv_source),
		"stream-type", 0,
		"format", GST_FORMAT_TIME, 
		"is-live", TRUE, NULL);

	/* Add Caps filter to Raw image to convert  BGR to YUV image format */
	GstCaps *caps = gst_caps_new_simple("video/x-raw", 
		"format", G_TYPE_STRING, "I420",/*Tis will change BGR image to I420*/
		"width", G_TYPE_INT, Width,
		"height", G_TYPE_INT, Height,
		"framerate", GST_TYPE_FRACTION, Framerate, 1,
		NULL);

	g_object_set(G_OBJECT(v_rawCapsfilter), "caps", caps, NULL);
	gst_caps_unref(caps);

	/* Set Caps filter for Encoder */
	GstCaps *vidEncCaps = gst_caps_new_simple("video/x-h264",
		"stream-format", G_TYPE_STRING, "byte-stream",
		"profile", G_TYPE_STRING,  "main",
		NULL);

	g_object_set(G_OBJECT(v_encCapsfilter), "caps", vidEncCaps, NULL);
	gst_caps_unref(vidEncCaps);

	g_object_set(G_OBJECT(encoder), "bitrate", 500000, "ref", 4, "pass", 4 , "key-int-max", 0, "byte-stream", TRUE, "tune", 0x00000004, "noise-reduction", 1000, NULL);

	/* Link all elements */
	if (gst_element_link_many(cv_source, v_convert, v_videorate, v_rawCapsfilter, encoder, v_encCapsfilter, v_queue ,NULL) != TRUE) {
		g_printerr("Many Elements could not be linked.\n");
		gst_object_unref(pipeline);
		return -1;
	}

	/* manual linking of video queue to muxer */
	LinkElementsManually(v_queue, mux); // Video Link 

	/* final link mux to filesink */
	if (gst_element_link_many(mux, filesink, NULL) != TRUE) {
		g_printerr("Many Elements could not be linked.\n");
		gst_object_unref(pipeline);
		return -1;
	}

	/* Set call back signal for get video data dinamically */
	g_signal_connect(cv_source, "need-data", G_CALLBACK(need_data_cv_image_data), NULL);

	/* Set the pipeline to "playing" state */
	if (gst_element_set_state(pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
	{
		g_printerr("Unable to set the pipeline to the playing state.\n");
		gst_object_unref(pipeline);
		getchar();
		return -1;
	}

	g_print("Running...\n");
	g_main_loop_run(loop);


	/* Free resources and change state to NULL */
	gst_object_unref(bus);
	g_print("Returned, stopping playback...\n");
	gst_element_set_state(pipeline, GST_STATE_NULL);
	g_print("Freeing pipeline...\n");
	gst_object_unref(GST_OBJECT(pipeline));
	g_print("Completed. Goodbye!\n");
	getchar();
	return 0;
}
