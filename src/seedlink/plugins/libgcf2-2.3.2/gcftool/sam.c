#if 0
/*
 * sam.c:
 *
 * Devived from code portions of which were
 * Copyright (c) 2001 Alessia Maggi <maggi@fishsoup.dhs.org>
 * 
 * Copyright (c) 2003 James McKenzie <software@guralp.com>
 * 
 * rights reserved.
 *
 * Modified and incorporated into jgcf 2001 James McKenzie
 *
 */

#include"project.h"

/* Sam disks are well behaved and organized */
/* if the user beleives otherwise she should */
/* extract the data as a dfd disk. */


#define BLOCK_SEARCH 100000     /*Distance to search for a relevant block */


G2Offt
find_block_of_stream (G2File * f, G2Offt n, int d, uint8_t * stream,
                      G2PBlock * pb, int tries)
{
  G2Block b;

  do
    {
      do
        {
          if (!(--tries))
            return -1;
        }
      while (G2FileRead1KBlock (f, n, &b));
      if (!memcmp (&b.data[4], stream, 4))
        {
          if (!G2ParseBlockHead (&b, (G2PBlockH *) pb))
            {
              return n;
            }
        }
      n += d;
    }
  while (1);
/*Keep anal compiler happy*/
  return -1;
}

/*Find the block which has a start time equal to t, otherwise err in the */
/*direction of d, if this is impossible return -1*/

// 08456005604

G2Offt
find_block_by_stime (G2File * f, G2Offt l, G2Offt h, int d,
                     uint8_t * stream, G2GTime * t)
{
  int m;
  G2PBlockH hh, hl, hm;

/* Check lower bound*/
  l = find_block_of_stream (f, l, 1, stream, (G2PBlockH *) & hl, BLOCKSEARCH);
  if (l < 0)
    return l;
  if (G2GTimeCompare (t, <, &hl->start))
    {
      return (d < 0) ? -1 : l;
    }

/* Check upper bound*/
  h =
    find_block_of_stream (f, h, -1, stream, (G2PBlockH *) & hh, BLOCKSEARCH);
  if (h < 0)
    return h;
  if (G2GTimeCompare (t, >, &hh->start))
    {
      return (d > 0) ? -1 : h;
    }

  do
    {
      m = ((l + h) / 2);
      m = find_block_of_stream (f, m, d, stream, (G2PBlockH *) & hm, 0);
      if ((m == h) || (m == l))
        {
          m = ((l + h) / 2);
          m = find_block_of_stream (f, m, -d, stream, (G2PBlockH *) & hm, 0);
        }
      if ((m == h) || (m == l))
        {
/*l and h are adjacent - we must choose one of them */
          if (d > 0)
            {
              if (G2GTimeCompare (&hl->start, = >, t))
                return l;
              if (G2GTimeCompare (&hh->start, = >, t))
                return h;
              return -1;
            `}
          else
            {
              if (G2GTimeCompare (&hh->start, <=, t))
                return h;
              if (G2GTimeCompare (&hl->start, <=, t))
                return l;
              return -1;
            }
        }

/*Decide to which of l and h we should assign m*/
      if (G2GTimeCompare (&hm->start, <, t))
        {
          l = m;
          hl->start = hm->start;
        }
      else if (G2GTimeCompare (&hm->start, >, t))
        {
          h = m;
          hh->start = hm->start;
        }
      else
        {
          return m;
        }
    }
  while (1);

}


G2Offt
find_block_by_etime (G2File * f, G2Offt l, G2Offt h, int d,
                     uint8_t * stream, G2GTime * t)
{
  int m;
  G2PBlockH hh, hl, hm;

/* Check lower bound*/
  l = find_block_of_stream (f, l, 1, stream, (G2PBlockH *) & hl, BLOCKSEARCH);
  if (l < 0)
    return l;
  if (G2GTimeCompare (t, <, &hl->end))
    {
      return (d < 0) ? -1 : l;
    }

/* Check upper bound*/
  h =
    find_block_of_stream (f, h, -1, stream, (G2PBlockH *) & hh, BLOCKSEARCH);
  if (h < 0)
    return h;
  if (G2GTimeCompare (t, >, &hh->end))
    {
      return (d > 0) ? -1 : h;
    }

  do
    {
      m = ((l + h) / 2);
      m = find_block_of_stream (f, m, d, stream, (G2PBlockH *) & hm, 0);
      if ((m == h) || (m == l))
        {
          m = ((l + h) / 2);
          m = find_block_of_stream (f, m, -d, stream, (G2PBlockH *) & hm, 0);
        }
      if ((m == h) || (m == l))
        {
/*l and h are adjacent - we must choose one of them */
          if (d > 0)
            {
              if (G2GTimeCompare (&hl->end, = >, t))
                return l;
              if (G2GTimeCompare (&hh->end, = >, t))
                return h;
              return -1;
            `}
          else
            {
              if (G2GTimeCompare (&hh->end, <=, t))
                return h;
              if (G2GTimeCompare (&hl->end, <=, t))
                return l;
              return -1;
            }
        }

/*Decide to which of l and h we should assign m*/
      if (G2GTimeCompare (&hm->end, <, t))
        {
          l = m;
          hl->end = hm->end;
        }
      else if (G2GTimeCompare (&hm->end, >, t))
        {
          h = m;
          hh->end = hm->end;
        }
      else
        {
          return m;
        }
    }
  while (1);

}

#if 0
void
check_blocks_for_gaps (Jgcf_disk d, int b, int b_end, char *stream_name,
                       Jgcf_utc * p_begin_time, int *p_n_samples,
                       int *p_s_rate)
{
  // FIXME: gaps across leap seconds will not be detected!
  time_t start_time, end_time;
  Jgcf_utc begin_time;
  int n_samples, s_rate;
  Jgcf_block block;

  n_samples = 0;

  block = jgcf_disk_read (d, b);

  s_rate = block->sample_rate;

  begin_time = jgcf_gtime_to_utc (block->start);

  end_time = jgcf_utc_to_time_t (jgcf_gtime_to_utc (block->start));

  free (block);

  if (!s_rate)
    {
      *p_begin_time = begin_time;
      *p_n_samples = n_samples;
      *p_s_rate = s_rate;
      return;
    }



  while (1)
    {

      block = jgcf_disk_read (d, b);



      //Assertions
      if (b > b_end)            // Have gone beyond end block
        crash_and_burn
          ("Have reached block %d which is after the last desirable block %d.\n",
           b, b_end);
      if (!block)
        crash_and_burn (" Failed to read block %d.\n", b);
      if (!block_is_integer_length (block))
        crash_and_burn (" Block %d has a non integer sample rate.\n", b);
      if (stream_mismatch_from_block (block, stream_name))
        crash_and_burn (" Stream mismatch in block %d %s!=%s.\n", b,
                        stream_name, block->strid);
      if (block->sample_rate != s_rate)
        crash_and_burn (" Sample rate mismatch in block %d.\n", b);


      start_time = jgcf_utc_to_time_t (jgcf_gtime_to_utc (block->start));
      if (start_time != end_time)
        {                       // Found a gap. 
          printf ("Found a gap just before block %d\n", b);
          printf ("End time of previous block was: %s\n",
                  my_ctime (&end_time));
          printf ("Start time of this block is   : %s\n",
                  my_ctime (&start_time));
          crash_and_burn ("You need to do this extraction breaking here");
        }


      end_time = find_end_time_t_from_block (block);

      n_samples += block->samples;

      if (b == b_end)
        {
          free (block);
          break;
        }
      b = find_next_block_number_from_block (block);

      free (block);
    }


  *p_begin_time = begin_time;
  *p_n_samples = n_samples;
  *p_s_rate = s_rate;
}
#endif

#if 0
Jgcf_utc
a_to_utc (char *a)
{
  Jgcf_utc ret;
  int i;
  char b[1024], *c;
  strcpy (b, a);

  c = strtok (b, ":/");
  if (!c)
    crash_and_burn ("julian day code failed");
  ret.hour = atoi (c);

  c = strtok (NULL, ":/");
  if (!c)
    crash_and_burn ("julian day code failed");
  ret.min = atoi (c);

  c = strtok (NULL, ":/");
  if (!c)
    crash_and_burn ("julian day code failed");
  ret.sec = atoi (c);

  c = strtok (NULL, ":/");
  if (!c)
    crash_and_burn ("julian day code failed");
  ret.day = atoi (c);

  c = strtok (NULL, ":/");
  if (!c)
    crash_and_burn ("julian day code failed");
  ret.mon = atoi (c);

  c = strtok (NULL, ":/");
  if (!c)
    crash_and_burn ("julian day code failed");
  ret.year = atoi (c);


  jgcf_fudge_jday (&ret);

  return ret;
}
#endif

/*Remaining requirements are to read directory*/

/*Walk datablocks checking for gaps overlaps and sample rate */
/*changes*/

/*Write into whatever our generic system requires - probably */
/*should write two extractors one for blocks t'other for */
/*streams*/

list_directory(G2File *f)
{
G2Offt b;
char buf[8192];

G2File1KRead


}



}



int
doextract (Jgcf_disk d, char *argv[], int sblock, int eblock, int usenums)
{
  int i, b_start, b_end;
  time_t t_start, t_end;
  char *stream;
  Jgcf_utc u, v;

  stream = argv[0];

  if ((!sblock) || (!eblock))
    crash_and_burn ("Stream not found on disk\n");

  printf ("Pre-bodge1  Blocks to search %d-%d\n", sblock, eblock);
  bodge_block (d, stream, &sblock, 2);
  bodge_block (d, stream, &eblock, -2);
  printf ("Post-bodge1 Blocks to search %d-%d\n", sblock, eblock);

  if (usenums)
    {
      b_start = atoi (argv[1]);
      b_end = atoi (argv[2]);
    }
  else
    {
      u = a_to_utc (argv[1]);
      v = a_to_utc (argv[2]);

      t_start = jgcf_utc_to_time_t (u);
      t_end = jgcf_utc_to_time_t (v);

      printf ("Start utc: ");
      jgcf_print_utc (u);
      printf ("End utc: ");
      jgcf_print_utc (v);

      b_start = find_block_from_date (d, sblock, eblock, u, stream, -1);
      b_end = find_block_from_date (d, sblock, eblock, v, stream, 1);
    }

  printf ("Pre-bodge2  Blocks to search %d-%d\n", sblock, eblock);
  bodge_block (d, stream, &sblock, 2);
  bodge_block (d, stream, &eblock, -2);
  printf ("Post-bodge2 Blocks to search %d-%d\n", sblock, eblock);

  t_start = find_time_t_from_block_number (d, b_start);
  t_end = find_time_t_from_block_number (d, b_end);

  printf ("Start block %d time %s\n", b_start, my_ctime (&t_start));
  printf ("End block %d time %s\n", b_end, my_ctime (&t_end));

  return really_do_extract (d, stream, b_start, b_end, 0);
}

int
doextractall (Jgcf_disk d, char *argv[], int sblock, int eblock)
{
  Jgcf_utc u, v, sac_begin_time;
  int i, b_start, b_end, b, b_new;
  time_t start_time, end_time, new_time;
  char *stream;
  Jgcf_block block;
  int rate;

  stream = argv[0];


  if ((!sblock) || (!eblock))
    crash_and_burn ("Stream not found on disk\n");

  printf ("Pre-bodge1  Blocks to search %d-%d\n", sblock, eblock);
  bodge_block (d, stream, &sblock, 2);
  bodge_block (d, stream, &eblock, -2);
  printf ("Post-bodge1 Blocks to search %d-%d\n", sblock, eblock);

  b_start = b_end = b = sblock;

  block = jgcf_disk_read (d, b);
  rate = block->sample_rate;
  if (rate)
    end_time = jgcf_utc_to_time_t (jgcf_gtime_to_utc (block->start));
  free (block);

  while (b < eblock)
    {
      int chop = 0;

      block = jgcf_disk_read (d, b);
      if (!block)
        {
          warning (" Failed to read block %d.\n", b);
          break;
        }

      if (!block_is_integer_length (block))
        {
          crash_and_burn (" Block %d has a non integer sample rate.\n", b);
          free (block);
          break;
        }
      if (stream_mismatch_from_block (block, stream))
        {
          crash_and_burn (" Stream mismatch in block %d %s!=%s.\n", b,
                          stream, block->strid);
          free (block);
          break;
        }

      start_time = jgcf_utc_to_time_t (jgcf_gtime_to_utc (block->start));
      if (rate)
        new_time = find_end_time_t_from_block (block);
      b_new = find_next_block_number_from_block (block);
      free (block);




      if (block->sample_rate != rate)
        {
          fprintf (stderr, "****ALL: Chopping because sample rate changed\n");
          chop++;
        }
      if (rate && (start_time != end_time))
        {
          fprintf (stderr, "****ALL: Chopping because of gap %d - %d\n",
                   start_time, end_time);
          chop++;
        }
      if ((((long int) new_time) % 3600) < (((long int) start_time) % 3600))
        {
          fprintf (stderr, "****ALL: Chopping because crossing hour\n");
          chop++;
        }


      if (chop)
        {
          fprintf (stderr, "****ALL: extracting from block %d-%d\n",
                   b_start, b_end);
          really_do_extract (d, stream, b_start, b_end, 1);
          b_start = b;
          end_time = start_time;

        }

      end_time = new_time;
      b_end = b;
      b = b_new;

    }

  fprintf (stderr, "****ALL: Chopping because have reached end of disk\n");
  fprintf (stderr, "****ALL: extracting from block %d-%d\n", b_start, b_end);

  really_do_extract (d, stream, b_start, b_end, 1);

}



void
usage (void)
{
  fprintf (stderr, "disk2sac [-d disc] [-b] [-s dstart] [-e dend] [-n]\n");
  fprintf (stderr, "             prints the disks directory\n");
  fprintf (stderr,
           "disk2sac [-d disc] [-b] [-s dstart] [-e dend] [-n] stream sstart send\n");
  fprintf (stderr, "     stream  the stream to extract\n");
  fprintf (stderr,
           "     sstart  time to start the extraction from the disk as \n");
  fprintf (stderr,
           "             hh:mm:ss/day/mon/yyyy or a block number is -n is set\n");
  fprintf (stderr,
           "     send    time to stop the extraction from the disk from as \n");
  fprintf (stderr,
           "             hh:mm:ss/day/mon/yyyy or a block number is -n is set\n");
  fprintf (stderr,
           "                if sstart or send do not correspond to stream, then\n");
  fprintf (stderr, "                they will be adjusted inwards\n");
  fprintf (stderr, "disk2sac [-d disc] [-b] [-s dstart] [-e dend] [-n] -i\n");
  fprintf (stderr,
           "             creates an index for the disk (streams times and\n");
  fprintf (stderr,
           "             block numbers) and writes it to index.dat (very very\n");
  fprintf (stderr, "             slow)\n");
  fprintf (stderr, "\n");
  fprintf (stderr,
           "disk2sac [-d disc] [-b] [-s dstart] [-e dend] -a stream\n");
  fprintf (stderr,
           "             chops and extracts all data from stream stream on the\n");
  fprintf (stderr, "             disk and writes them in to files.\n");
  fprintf (stderr, "\n");
  fprintf (stderr,
           "     -d Specifies the disk to use (either a directory or a device)\n");
  fprintf (stderr,
           "     -b indicates that the disk directory is bad and that disk2sac\n");
  fprintf (stderr,
           "        should attempt this extraction without it (slow)\n");
  fprintf (stderr,
           "     -s optionally specify the first data block on the disk\n");
  fprintf (stderr,
           "        corressponding to the specified stream, if not specified\n");
  fprintf (stderr,
           "        disk2sac will either use the directory or if -b is also\n");
  fprintf (stderr,
           "        specified disk2sac will attempt to find it (slow)\n");
  fprintf (stderr,
           "     -e optionally specify the last data block corressponding to \n");
  fprintf (stderr, "        the specified stream. (see notes for -s)\n");
  fprintf (stderr,
           "     -n indicates that sstart and send are block numbers not times\n");
  fprintf (stderr, "\n");
  fprintf (stderr,
           "        A default disk device can be set with the environment\n");
  fprintf (stderr, "        variable GCFDISK\n");

  exit (1);
}

int
main (int argc, char *argv[])
{
  Jgcf_disk d;

  int sblock = 0, eblock = 0;
  int blocknums = 0;
  int bodge = 0;
  int doindex = 0;
  int doeverything = 0;

  char *disc = getenv ("GCFDISK") ? getenv ("GCFDISK") : "/dev/sda";
  int c;

  while ((c = getopt (argc, argv, "ad:bs:e:nih")) != EOF)
    {
      switch (c)
        {
        case 'b':
          bodge++;
          break;
        case 'd':
          disc = optarg;
          break;
        case 's':
          sblock = atoi (optarg);
          break;
        case 'e':
          eblock = atoi (optarg);
          break;
        case 'n':
          blocknums++;
          break;
        case 'i':
          doindex++;
          break;
        case 'a':
          doeverything++;
          break;
        case '?':
        case 'h':
          usage ();
        }
    }

  d = jgcf_disk_open (disc);

  if (!d)
    {
      fprintf (stderr, "Failed to open disk %s\n", disc);

      usage ();
      exit (1);
    }

  argv += optind;
  argc -= optind;





  if (doindex)
    {
      return make_index (d);
    }

  jgcf_disk_dir (d);

  if (!argc)
    return;

  if ((argc != 3) && (!doeverything))
    usage ();
  if ((argc != 1) && (doeverything))
    usage ();

  if (!bodge)
    {
      if (!sblock)
        sblock = jgcf_disk_first_block (d, argv[0]);
      if (!eblock)
        eblock = jgcf_disk_last_block (d, argv[0]);
    }
  else
    {
      if (!sblock)
        sblock = bodge_bottom (d);
      if (!eblock)
        eblock = bodge_top (d);
    }

  if (doeverything)
    return doextractall (d, argv, sblock, eblock);

  return doextract (d, argv, sblock, eblock, blocknums);
}
#endif
