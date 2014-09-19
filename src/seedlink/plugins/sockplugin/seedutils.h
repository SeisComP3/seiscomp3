/*
 * seedutils.h
 * Declarations for the seedutils.c routines
 */

/* Some SEED structures */

#include <stdint.h>

#define PACKED __attribute__ ((packed))

typedef struct s_btime {      /* RAW TIME IN FSDH */
  uint16_t year;
  uint16_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t secs;
  uint8_t unused;
  uint16_t fracts;
} PACKED t_btime;

typedef struct s_dev_id {     /* DEVICE ID IN FSDH */
  char station_code[5];
  char location_id[2];
  char channel_id[3];
  char network_code[2];
} PACKED t_dev_id;

typedef struct s_fsdh_data {  /* FIXED-SECTION-DATA-HEADER DATA */
  struct s_btime start_time;
  uint16_t num_samples;
  short sample_rate;
  short multiplier;
  uint8_t activity_flags;
  uint8_t io_flags; 
  uint8_t dq_flags;
  uint8_t num_blockettes;
  int32_t time_correct;
  uint16_t begin_data;
  uint16_t begin_blockette;
} PACKED t_fsdh_data;

typedef struct s_blk_head {  /* generic struct for head of blockettes */
  uint16_t blk_type;
  uint16_t next_blk;
} PACKED t_blk_head;

typedef struct s_blk_1000 {  /* BLOCKETTE 1000 */
  uint16_t blk_type;
  uint16_t next_blk;
  uint8_t encoding;
  uint8_t word_swap;
  uint8_t rec_len;
  uint8_t reserved;
} PACKED t_blk_1000;


/* Function declarations for routines in seedutil.c */

int process_telemetry ( char *host, int port, unsigned char *buf,
                        char *match, char *netcode, int rec_size );
void process_record ( char *inseed, char *match,
		      char *netcode, int rec_size );
int parse_record ( t_dev_id *dev_id, 
		   t_blk_1000 *blk_1000, 
		   char *lptr, int rec_size );
void swap_2bytes ( uint16_t *a, char f );
void swap_4bytes ( uint32_t *a, char f );
int matches ( char *source, char *match );
char *encoding_hash( char enc );
