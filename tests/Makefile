MODULE         := pmm
SUBMODULE      := multi
NAME           := $(MODULE)_$(SUBMODULE)
SRCS           := framework/$(MODULE)/$(SUBMODULE).c $(shell find -L ./src/ -name "*.c")
INC_PATH       := include/ framework/include/

export AM_HOME := $(PWD)/../abstract-machine
ifeq ($(ARCH),)
export ARCH    := native
endif

include $(AM_HOME)/Makefile
include ../Makefile.lab
image:
