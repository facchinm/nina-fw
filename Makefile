PROJECT_NAME := nina-fw

EXTRA_COMPONENT_DIRS := $(PWD)/arduino

ifeq ($(RELEASE),1)
CFLAGS += -DNDEBUG -DCONFIG_FREERTOS_ASSERT_DISABLE -Os -DLOG_LOCAL_LEVEL=0
CPPFLAGS += -DNDEBUG -Os
endif

include $(IDF_PATH)/make/project.mk

firmware: all makefs
	python combine.py

.PHONY: firmware
