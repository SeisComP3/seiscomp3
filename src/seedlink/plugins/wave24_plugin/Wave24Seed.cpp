#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <getopt.h>
#include <termios.h>
#include <math.h>

#include "plugin.h"

#define _WAVE24SEED_GLOBALS

#include "Wave24Seed.h"

int32_t WaveSeed::getInt32(char *pos) {
  if (!pos) return 0;
  
  char field[4];
  
  for (int i=0;i<4;i++) {
    field[4-1-i]=*(pos+i);
  }
  
  int32_t *valueField = (int32_t *) field;
  int32_t value = *valueField;
  
  return value;
}

//pos sa rata od 0!!!
int32_t WaveSeed::getNibble(int32_t nibble,int pos) {
  int32_t mask=0x3;
  if (pos>15) return -1;
  
  mask=mask<<((14-pos)*2);
  nibble&=mask;
  nibble=nibble>>((14-pos)*2);
  
  return nibble;
}

int32_t WaveSeed::getDNibble(int32_t value) {
  int32_t mask=0xC0000000;
  
  value&=mask;
  value=value>>30;
  value&=0x3;
  
  return value;
}
  
int32_t WaveSeed::getOne(int32_t value){
  value&=0x3FFFFFFF;
  if (value>0x1FFFFFFF) value=value-0x40000000;
  return value;
}

int32_t WaveSeed::getTwo(int32_t value,int pos) {
  //>=-16384,<-512;>511,<=16383
  if (pos<0 || pos>1) return 0;
  int32_t mask=0x7FFF;
  mask=mask<<((1-pos)*15);
  value&=mask;
  value=value>>((1-pos)*15);
  value&=0x7FFF;
  if (value>16383) value=value-32768;
  return value;
}

int32_t WaveSeed::getThree(int32_t value,int pos) {
  //>=-512,<-128;>127,<=511
  if (pos<0 || pos>2) return 0;
  int32_t mask=0x3FF;
  mask=mask<<((2-pos)*10);
  value&=mask;
  value=value>>((2-pos)*10);
  value&=0x3FF;
  if (value>511) value=value-1024;
  return value;
}

int32_t WaveSeed::getFour(int32_t value,int pos){
  //>=-128,<-32;>31,<=127
  if (pos<0 || pos>3) return 0;
  int32_t mask=0xFF;
  mask=mask<<((3-pos)*8);
  value&=mask;
  value=value>>((3-pos)*8);
  value&=0xFF;
  if (value>127) value=value-256;
  return value;
}

int32_t WaveSeed::getFive(int32_t value,int pos) {
  //>=-32,<-16;>15,<=31
  if (pos<0 || pos>4) return 0;
  int32_t mask=0x3F;
  mask=mask<<((4-pos)*6);
  value&=mask;
  value=value>>((4-pos)*6);
  value&=0x3F;
  if (value>31) value=value-64;
  return value;
}

int32_t WaveSeed::getSix(int32_t value,int pos) {
  //>=-16,<-8;>7,<=15
  if (pos<0 || pos>5) return 0;
  int32_t mask=0x1F;
  mask=mask<<((5-pos)*5);
  value&=mask;
  value=value>>((5-pos)*5);
  value&=0x1F;
  if (value>15) value=value-32;
  return value;
}

int32_t WaveSeed::getSeven(int32_t value,int pos) {
  //>=-8,<=7
  if (pos<0 || pos>6) return 0;
  int32_t mask=0xF;
  mask=mask<<((6-pos)*4);
  value&=mask;
  value=value>>((6-pos)*4);
  value&=0xF;
  if (value>7) value=value-16;
  return value;
}

//recordy su uvazovane vzdy 512B a data sa zacinaju od 64
int WaveSeed::decodeSteim(char *record,int32_t* samples) {
  int nsamples=0;
  int i;
  char tmpstring[40];

  //WDB printf("\n-------------------------- Original ---------------------------\n");
  //if (fp>=0) {
  //  WDB printf("Writing original\n");
  //  WDB write(fp,(void *) record,512);
  //}
  
  //WDB printf("\n--------------------------- Decode ------------------------------\n");
  
  for (i=0;i<7;i++) {
    //WDB printf("Decode seed %d\n",i);
    char *pos=record+64+i*64;
    
    int32_t nibble=getInt32(pos);
    //WDB printf("Nibble: %X\n",nibble);
    int32_t value;
    int32_t dnibble;
    int32_t posNibble;
    for (int j=0;j<15;j++) {
      pos+=4;
      value=getInt32(pos);
      posNibble=getNibble(nibble,j);
      //WDB printf("PosNIbble: %X\n",posNibble);
      switch (posNibble) {
        case NODATA:
    if (j==0 && i==0 ) {
      samples[nsamples++]=value;
      //WDB sprintf(tmpstring,"First sample %d",value);
      //WDB driver->proto('m',tmpstring);
    }
    if (j==1 && i==0 ) {
      //WDB sprintf(tmpstring,"Last sample read %d",value);
      //WDB driver->proto('m',tmpstring);
    }
          break;
        case FOUR:
          for (int k=0;k<4;k++) {
      samples[nsamples++]=getFour(value,k);
          }
          break;
        case DNIB1:
          dnibble=getDNibble(value);
    //WDB printf("Value %X, Dnibble %x\n",value,dnibble);
    switch (dnibble) {
      case DNIB1_ONE:
        for(int k=0;k<1;k++) {
          samples[nsamples++]=getOne(value);
        }
        break;
      case DNIB1_TWO:
        for (int k=0;k<2;k++) {
          samples[nsamples++]=getTwo(value,k);
        }
        break;
      case DNIB1_THREE:
        for (int k=0;k<3;k++) {
          samples[nsamples++]=getThree(value,k);
              }
        break;
    }
          break;
        case DNIB2:
    dnibble=getDNibble(value);
    //WDB printf("Value %X, Dnibble %x\n",value,dnibble);
    switch (dnibble) {
      case DNIB2_FIVE:
        for (int k=0;k<5;k++) {
          samples[nsamples++]=getFive(value,k);
              }
        break;
      case DNIB2_SIX:
        for (int k=0;k<6;k++) {
          samples[nsamples++]=getSix(value,k);
              }
        break;
      case DNIB2_SEVEN:
        for (int k=0;k<7;k++) {
          samples[nsamples++]=getSeven(value,k);
              }
        break;
    }
          break;
      }
    
    }
  }
  
  //for (i=0;i<nsamples;i++) {
  //  printf("%d\n",samples[i]);
  //}
  //WDB printf("\n-------------------------------------------end------------------------\n");
  
  return nsamples;  
}

int WaveSeed::getFirstTime(char *record,struct ptime *firstTime){
  if (!record) return 0;
  
  char cyear[2];
  cyear[0]=*(record+21);
  cyear[1]=*(record+20);
  
  char cday[2];
  cday[0]=*(record+23);
  cday[1]=*(record+22);
  
  char hour=*(record+24);
  char min=*(record+25);
  char sec=*(record+26);
  
  char cdms[2];
  cdms[0]=*(record+29);
  cdms[1]=*(record+28);
  
  int16_t year=*((int16_t *) cyear);
  int16_t day=*((int16_t *) cday);
  int16_t dms=*((int16_t *) cdms);
  
  firstTime->year=year;
  firstTime->yday=day;
  firstTime->hour=hour;
  firstTime->minute=min;
  firstTime->second=sec;
  firstTime->usec=dms*100;
  
  return 1; 
}
    
int WaveSeed::clockOk(char *record){
  int result=0;
  if (!record) return 0;
   
  char firstFlag=*(record+37);
  char secondFlag=*(record+38);
  
  if (!(firstFlag&32)) result |= GPS_NOT_LOCKED;
  if ( secondFlag&128 ) result |= QUESTIONABLE_TIME_TAG;
  return result;
}
