/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2001             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: utility functions for loading .vqh and .vqd files
 last mod: $Id: bookutil.c 16037 2009-05-26 21:10:58Z xiphmont $

 ********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include "bookutil.h"
#include <sys\timeb.h>

int gettimeofday (struct timeval *tp, void *tz)
{
	struct _timeb timebuffer;
	_ftime64_s (&timebuffer);
	tp->tv_sec = (long)timebuffer.time;
	tp->tv_usec = timebuffer.millitm * 1000;
	return 0;
}

/* A few little utils for reading files */
/* read a line.  Use global, persistent buffering */
static char *linebuffer=NULL;
static int  lbufsize=0;
char *get_line(FILE *in){
  long sofar=0;
  if(feof(in))return NULL;

  while(1){
    int gotline=0;

    while(!gotline){
      if(sofar+1>=lbufsize){
        if(!lbufsize){  
          lbufsize=1024;
          linebuffer=_ogg_malloc(lbufsize);
        }else{
          lbufsize*=2;
          linebuffer=_ogg_realloc(linebuffer,lbufsize);
        }
      }
      {
        long c=fgetc(in);
        switch(c){
        case EOF:
          if(sofar==0)return(NULL);
          /* fallthrough correct */
        case '\n':
          linebuffer[sofar]='\0';
          gotline=1;
          break;
        default:
          linebuffer[sofar++]=(char)c;
          linebuffer[sofar]='\0';
          break;
        }
      }
    }
    
    if(linebuffer[0]=='#'){
      sofar=0;
    }else{
      return(linebuffer);
    }
  }
}

/* read the next numerical value from the given file */
static char *value_line_buff=NULL;

int get_line_value(FILE *in,float *value){
  char *next;

  if(!value_line_buff)return(-1);

  *value=(float)strtod(value_line_buff, &next);
  if(next==value_line_buff){
    value_line_buff=NULL;
    return(-1);
  }else{
    value_line_buff=next;
    while(*value_line_buff>44)value_line_buff++;
    if(*value_line_buff==44)value_line_buff++;
    return(0);
  }
}

int get_next_value(FILE *in,float *value){
  while(1){
    if(get_line_value(in,value)){
      value_line_buff=get_line(in);
      if(!value_line_buff)return(-1);
    }else{
      return(0);
    }
  }
}

int get_next_ivalue(FILE *in,long *ivalue){
  float value;
  int ret=get_next_value(in,&value);
  *ivalue=(long)value;
  return(ret);
}

static float sequence_base=0.f;
static int v_sofar=0;
void reset_next_value(void){
  value_line_buff=NULL;
  sequence_base=0.f;
  v_sofar=0;
}

char *setup_line(FILE *in){
  reset_next_value();
  value_line_buff=get_line(in);
  return(value_line_buff);
}


int get_vector(codebook *b,FILE *in,int start, int n,float *a){
  int i;
  const static_codebook *c=b->c;

  while(1){

    if(v_sofar==n || get_line_value(in,a)){
      reset_next_value();
      if(get_next_value(in,a))
        break;
      for(i=0;i<start;i++){
        sequence_base=*a;
        get_line_value(in,a);
      }
    }

    for(i=1;i<c->dim;i++)
      if(get_line_value(in,a+i))
        break;
    
    if(i==c->dim){
      float temp=a[c->dim-1];
      for(i=0;i<c->dim;i++)a[i]-=sequence_base;
      if(c->q_sequencep)sequence_base=temp;
      v_sofar++;
      return(0);
    }
    sequence_base=0.f;
  }

  return(-1);
}

/* read lines fromt he beginning until we find one containing the
   specified string */
char *find_seek_to(FILE *in,char *s){
  rewind(in);
  while(1){
    char *line=get_line(in);
    if(line){
      if(strstr(line,s))
        return(line);
    }else
      return(NULL);
  }
}
