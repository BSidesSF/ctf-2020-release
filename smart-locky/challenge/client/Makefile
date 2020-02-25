#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := client
ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

-include $(IDF_PATH)/make/project.mk

docker-build:
	docker run --rm -v $(ROOT_DIR):/project -w /project espressif/idf make defconfig all -j4

docker-clean:
	docker run --rm -v $(ROOT_DIR):/project -w /project espressif/idf make clean

docker-flash:
	docker run --rm -v $(ROOT_DIR):/project -w /project --device=/dev/ttyUSB0 espressif/idf make flash
