#!/bin/bash

HYPO71PC_BINARY=h71.bin
# Jumping into the right directory
cd ${HOME}/hypo71/

# Executing binary with input file as argument
./$HYPO71PC_BINARY < input

