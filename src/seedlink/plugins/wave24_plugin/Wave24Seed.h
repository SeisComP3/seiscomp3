#ifndef _WAVE24SEED
#define _WAVE24SEED

class WaveSeed {
  public:
    static int decodeSteim(char *record,int32_t* samples);
    static int getFirstTime(char *record,struct ptime *firstTime);
    static int clockOk(char *record);
    static const int GPS_NOT_LOCKED=0x01;
    static const int QUESTIONABLE_TIME_TAG=0x02;
  private:
    static int32_t getInt32(char *pos);
    static int32_t getNibble(int32_t nibble,int pos);
    static int32_t getDNibble(int32_t value);
    static int32_t getOne(int32_t value);
    static int32_t getTwo(int32_t value,int pos);
    static int32_t getThree(int32_t value,int pos);
    static int32_t getFour(int32_t value,int pos);
    static int32_t getFive(int32_t value,int pos);
    static int32_t getSix(int32_t value,int pos);
    static int32_t getSeven(int32_t value,int pos);
    static const int NODATA=0;
    static const int FOUR=1;
    static const int DNIB1=2;
    static const int DNIB2=3;
    static const int DNIB1_ONE=1;
    static const int DNIB1_TWO=2;
    static const int DNIB1_THREE=3;
    static const int DNIB2_FIVE=0;
    static const int DNIB2_SIX=1;
    static const int DNIB2_SEVEN=2;
};

#endif

