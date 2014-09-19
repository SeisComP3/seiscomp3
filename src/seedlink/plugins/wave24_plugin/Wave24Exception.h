#ifndef _WAVE24EXCEPTION
#define _WAVE24EXCEPTION

class Wave24Exception {
  private:
    char message[1024];
    bool canRun;

  public:
    Wave24Exception(char *message,bool canRun) {
      strncpy(this->message,message,1024);
      this->canRun=canRun;
    }
    Wave24Exception(bool canRun) {
      strcpy(this->message,"Unknown reason");
      this->canRun=canRun;
    }
    Wave24Exception() {
      strcpy(this->message,"Unknown reason");
      this->canRun=false;
    }
    char* describe() {
      return message;
    }
    bool canContinue() {
      return canRun;
    }
};

#endif


