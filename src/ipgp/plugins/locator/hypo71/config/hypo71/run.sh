#!/bin/bash

HYPO71PC_BINARY=Hypo71PC
HYPO71PC_HOME=`dirname $0`

# Jumping into the right directory
cd ${HYPO71PC_HOME}/

# Executing binary with input file as argument
./$HYPO71PC_BINARY < input

