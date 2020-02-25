#
# Main component makefile.
#
# This Makefile can be left empty. By default, it will take the sources in the 
# src/ directory, compile them and link them into lib(subdirectory_name).a 
# in the build directory. This behaviour is entirely configurable,
# please read the ESP-IDF documents if you need to do this.
#
#

#COMPONENT_EXTRA_CLEAN := secrets.c
COMPONENT_OBJINCLUDE := secrets.o

secrets.o: secrets.c

secrets.c: $(COMPONENT_PATH)/generate_keys.py $(COMPONENT_PATH)/include/locky_secrets.h
	python3 $(COMPONENT_PATH)/generate_keys.py > $(COMPONENT_PATH)/$@
