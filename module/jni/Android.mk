LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := logmute
LOCAL_SRC_FILES := main.cpp nopFun.cpp companion.cpp pmparser.c
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
