LOCAL_PATH			:= $(call my-dir)
SRC_PATH			:= ../..
COMMON_PATH			:= $(SRC_PATH)/../Common
COMMON_INC_PATH		:= $(COMMON_PATH)/Include
COMMON_SRC_PATH		:= $(COMMON_PATH)/Source

include $(CLEAR_VARS)

LOCAL_MODULE    := Point2Line
LOCAL_CFLAGS    += -DANDROID


LOCAL_SRC_FILES := $(COMMON_SRC_PATH)/esShader.c \
				   $(COMMON_SRC_PATH)/esShapes.c \
				   $(COMMON_SRC_PATH)/esTransform.c \
				   $(COMMON_SRC_PATH)/esUtil.c \
				   $(COMMON_SRC_PATH)/Android/esUtil_Android.c \
				   $(SRC_PATH)/Point2Line.c


LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../Tools/Image/png
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../Tools/Image/zlib
#source code
LOCAL_C_INCLUDES += $(SRC_PATH) \
					../../../External/Include \
					../../../Common/Include

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv3

LOCAL_STATIC_LIBRARIES := libpng
LOCAL_STATIC_LIBRARIES += libz
LOCAL_STATIC_LIBRARIES += android_native_app_glue


include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

include $(LOCAL_PATH)/../../../Tools/Image/png/Android.mk $(LOCAL_PATH)/../../../Tools/Image/zlib/Android.mk

$(call import-module,android/native_app_glue)


