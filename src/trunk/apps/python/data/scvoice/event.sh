#!/bin/sh

echo " $1" 

if [ "$2" = "1" ]; then
	aplay ~/.seiscomp3/siren03.wav
	echo " $1" | sed 's/,/, ,/g'   | festival --tts;
else
	echo " $1" | sed 's/,/, ,/g'   | festival --tts;
fi

