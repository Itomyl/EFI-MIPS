#
# Copyright (c) 1999, 2000
# Intel Corporation.
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# 3. All advertising materials mentioning features or use of this software must
#    display the following acknowledgement:
# 
#    This product includes software developed by Intel Corporation and its
#    contributors.
# 
# 4. Neither the name of Intel Corporation or its contributors may be used to
#    endorse or promote products derived from this software without specific
#    prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 

#
# Include sdk.env environment
#

include $(SDK_INSTALL_DIR)/build/$(SDK_BUILD_ENV)/sdk.env

#
# Set the base output name
#

BASE_NAME = Parser

#
# Globals needed by maseter.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)/cmds/python/$(BASE_NAME)
BUILD_DIR  = $(SDK_INSTALL_DIR)/cmds/python/$(BASE_NAME)

#
# Additional compile flags
#

#C_FLAGS += -D __STDC__

#
# Include paths
#

include $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/$(PROCESSOR)

include $(SDK_INSTALL_DIR)/include/bsd/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/bsd

include ../include/makefile.hdr
INC += -I ../include

#
# Default target
#

all : $(OBJECTS)

#
# Local include dependencies
#

INC_DEPS +=  \
	parser.h \
	tokenizer.h 

#
# Library object files
#

OBJECTS +=  \
    $(BUILD_DIR)/acceler.o \
    $(BUILD_DIR)/bitset.o \
    $(BUILD_DIR)/grammar1.o \
    $(BUILD_DIR)/intrcheck.o \
    $(BUILD_DIR)/listnode.o \
    $(BUILD_DIR)/metagrammar.o \
    $(BUILD_DIR)/myreadline.o \
    $(BUILD_DIR)/node.o \
    $(BUILD_DIR)/parser.o \
    $(BUILD_DIR)/parsetok.o \
    $(BUILD_DIR)/tokenizer.o 

#
# Source file dependencies
#

$(BUILD_DIR)/acceler.o     :  $(INC_DEPS)
$(BUILD_DIR)/grammar1.o    :  $(INC_DEPS)
$(BUILD_DIR)/listnode.o    :  $(INC_DEPS)
$(BUILD_DIR)/node.o        :  $(INC_DEPS)
$(BUILD_DIR)/parser.o      :  $(INC_DEPS)
$(BUILD_DIR)/parsetok.o    :  $(INC_DEPS)
$(BUILD_DIR)/tokenizer.o   :  $(INC_DEPS)
$(BUILD_DIR)/bitset.o      :  $(INC_DEPS)
$(BUILD_DIR)/metagrammar.o :  $(INC_DEPS)
$(BUILD_DIR)/myreadline.o  :  $(INC_DEPS)
$(BUILD_DIR)/intrcheck.o   :  $(INC_DEPS)

#
# Handoff to master.mak
#

include $(SDK_INSTALL_DIR)/build/master.mak
