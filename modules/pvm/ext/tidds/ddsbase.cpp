/*
 * Copyright (c) by Stefan Roettger
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * From the V^3 volume renderer <http://stereofx.org/volume.html>, license
 * change to LGPL with kind permission from Stefan Roettger
 */

#include "ddsbase.h"

#ifdef WIN32
#  define snprintf _snprintf
#endif

#define DDS_MAXSTR (256)

#define DDS_BLOCKSIZE (1<<20)
#define DDS_INTERLEAVE (1<<24)

#define DDS_RL (7)

FILE *DDS_file;

char DDS_ID[]="DDS v3d\n";
char DDS_ID2[]="DDS v3e\n";

unsigned int DDS_buffer;
int DDS_bufsize,DDS_bitcnt;

unsigned short int DDS_INTEL=1;

unsigned int checksum(unsigned char *data,unsigned int bytes);

inline unsigned int DDS_shiftl(const unsigned int value,const int bits)
   {return((bits>=32)?0:value<<bits);}

inline unsigned int DDS_shiftr(const unsigned int value,const int bits)
   {return((bits>=32)?0:value>>bits);}

static void initbuffer()
   {
   DDS_buffer=0;
   DDS_bufsize=0;
   DDS_bitcnt=0;
   }

static void DDS_swapuint(unsigned int *x)
   {
   unsigned int tmp=*x;

   *x=((tmp&0xff)<<24)|
      ((tmp&0xff00)<<8)|
      ((tmp&0xff0000)>>8)|
      ((tmp&0xff000000)>>24);
   }

static void writebits(FILE *file,unsigned int value,int bits)
   {
   if (bits<0 || bits>32) ERRORMSG();

   if (bits==0) return;

   value&=DDS_shiftl(1,bits)-1;

   if (DDS_bufsize+bits<32)
      {
      DDS_buffer=DDS_shiftl(DDS_buffer,bits)|value;
      DDS_bufsize+=bits;
      }
   else
      {
      DDS_buffer=DDS_shiftl(DDS_buffer,32-DDS_bufsize);
      DDS_bufsize+=bits-32;
      DDS_buffer|=DDS_shiftr(value,DDS_bufsize);
      DDS_swapuint(&DDS_buffer);
      if (fwrite(&DDS_buffer,4,1,file)!=1) ERRORMSG();
      DDS_buffer=value&(DDS_shiftl(1,DDS_bufsize)-1);
      }

   DDS_bitcnt+=bits;
   }

static void flushbits(FILE *file)
   {
   if (DDS_bufsize>0)
      {
      DDS_buffer=DDS_shiftl(DDS_buffer,32-DDS_bufsize);
      DDS_swapuint(&DDS_buffer);
      if (fwrite(&DDS_buffer,(DDS_bufsize+7)/8,1,file)!=1) ERRORMSG();
      DDS_bitcnt+=(32-DDS_bufsize)&7;
      }
   }

static unsigned int readbits(FILE *file,int bits)
   {
   unsigned int value;

   if (bits<0 || bits>32) ERRORMSG();

   if (bits==0) return(0);

   if (bits<DDS_bufsize)
      {
      DDS_bufsize-=bits;
      value=DDS_shiftr(DDS_buffer,DDS_bufsize);
      }
   else
      {
      value=DDS_shiftl(DDS_buffer,bits-DDS_bufsize);
      DDS_buffer=0;
      if( fread(&DDS_buffer,1,4,file) <= 0 )
        DDS_buffer=0;
      else
        DDS_swapuint(&DDS_buffer);
      DDS_bufsize+=32-bits;
      value|=DDS_shiftr(DDS_buffer,DDS_bufsize);
      }

   DDS_buffer&=DDS_shiftl(1,DDS_bufsize)-1;
   DDS_bitcnt+=bits;

   return(value);
   }

inline int DDS_code(int bits)
   {return(bits>1?bits-1:bits);}

inline int DDS_decode(int bits)
   {return(bits>=1?bits+1:bits);}

// deinterleave a byte stream
static void deinterleave(unsigned char *data,size_t bytes,size_t skip,size_t block=0,BOOLINT restore=FALSE)
   {
   size_t i,j,k;

   unsigned char *data2,*ptr;

   if (skip<=1) return;

   if (block==0)
      {
      if ((data2=(unsigned char *)malloc(bytes))==NULL) ERRORMSG();

      if (!restore)
         for (ptr=data2,i=0; i<skip; i++)
            for (j=i; j<bytes; j+=skip) *ptr++=data[j];
      else
         for (ptr=data,i=0; i<skip; i++)
            for (j=i; j<bytes; j+=skip) data2[j]=*ptr++;

      memcpy(data,data2,bytes);
      }
   else
      {
      if ((data2=(unsigned char *)malloc((bytes<skip*block)?bytes:skip*block))==NULL) ERRORMSG();

      if (!restore)
         {
         for (k=0; k<bytes/skip/block; k++)
            {
            for (ptr=data2,i=0; i<skip; i++)
               for (j=i; j<skip*block; j+=skip) *ptr++=data[k*skip*block+j];

            memcpy(data+k*skip*block,data2,skip*block);
            }

         for (ptr=data2,i=0; i<skip; i++)
            for (j=i; j<bytes-k*skip*block; j+=skip) *ptr++=data[k*skip*block+j];

         memcpy(data+k*skip*block,data2,bytes-k*skip*block);
         }
      else
         {
         for (k=0; k<bytes/skip/block; k++)
            {
            for (ptr=data+k*skip*block,i=0; i<skip; i++)
               for (j=i; j<skip*block; j+=skip) data2[j]=*ptr++;

            memcpy(data+k*skip*block,data2,skip*block);
            }

         for (ptr=data+k*skip*block,i=0; i<skip; i++)
            for (j=i; j<bytes-k*skip*block; j+=skip) data2[j]=*ptr++;

         memcpy(data+k*skip*block,data2,bytes-k*skip*block);
         }
      }

   free(data2);
   }

// interleave a byte stream
static void interleave(unsigned char *data,size_t bytes,unsigned int skip,unsigned int block=0)
   {deinterleave(data,bytes,skip,block,TRUE);}

// write a Differential Data Stream
void writeDDSfile(const char *filename,unsigned char *data,size_t bytes,unsigned int skip,unsigned int strip,int nofree)
   {
   int version=1;

   unsigned char *ptr1,*ptr2;

   int pre1,pre2,
       act1,act2,
       tmp1,tmp2;

   unsigned int cnt,cnt1,cnt2;
   int bits,bits1,bits2;

   if (bytes<1) ERRORMSG();

   if (bytes>DDS_INTERLEAVE) version=2;

   if (skip<1 || skip>4) skip=1;
   if (strip<1 || strip>65536) strip=1;

   if ((DDS_file=fopen(filename,"wb"))==NULL) ERRORMSG();

   fprintf(DDS_file,"%s",(version==1)?DDS_ID:DDS_ID2);

   deinterleave(data,bytes,skip,DDS_INTERLEAVE);

   initbuffer();

   writebits(DDS_file,skip-1,2);
   writebits(DDS_file,strip++-1,16);

   ptr1=ptr2=data;
   pre1=pre2=0;

   cnt=cnt1=cnt2=0;
   bits=bits1=bits2=0;

   while (cnt++<bytes)
      {
      tmp1=*ptr1++;
      if (cnt<=strip) act1=tmp1-pre1;
      else act1=tmp1-pre1-*(ptr1-strip)+*(ptr1-strip-1);
      pre1=tmp1;

      while (act1<-128) act1+=256;
      while (act1>127) act1-=256;

      if (act1<=0)
         for (bits=0; (1<<bits)/2<-act1; bits++) ;
      else
         for (bits=0; (1<<bits)/2<=act1; bits++) ;

      bits=DDS_decode(DDS_code(bits));

      if (cnt1==0)
         {
         cnt1++;
         bits1=bits;
         continue;
         }

      if (cnt1<(1<<DDS_RL)-1 && bits==bits1)
         {
         cnt1++;
         continue;
         }

      if (cnt1+cnt2<(1<<DDS_RL) && (cnt1+cnt2)*max(bits1,bits2)<cnt1*bits1+cnt2*bits2+DDS_RL+3)
         {
         cnt2+=cnt1;
         if (bits1>bits2) bits2=bits1;
         }
      else
         {
         writebits(DDS_file,cnt2,DDS_RL);
         writebits(DDS_file,DDS_code(bits2),3);

         while (cnt2-->0)
            {
            tmp2=*ptr2++;
            if (ptr2-strip<=data) act2=tmp2-pre2;
            else act2=tmp2-pre2-*(ptr2-strip)+*(ptr2-strip-1);
            pre2=tmp2;

            while (act2<-128) act2+=256;
            while (act2>127) act2-=256;

            writebits(DDS_file,act2+(1<<bits2)/2,bits2);
            }

         cnt2=cnt1;
         bits2=bits1;
         }

      cnt1=1;
      bits1=bits;
      }

   if (cnt1+cnt2<(1<<DDS_RL) && (cnt1+cnt2)*max(bits1,bits2)<cnt1*bits1+cnt2*bits2+DDS_RL+3)
      {
      cnt2+=cnt1;
      if (bits1>bits2) bits2=bits1;
      }
   else
      {
      writebits(DDS_file,cnt2,DDS_RL);
      writebits(DDS_file,DDS_code(bits2),3);

      while (cnt2-->0)
         {
         tmp2=*ptr2++;
         if (ptr2-strip<=data) act2=tmp2-pre2;
         else act2=tmp2-pre2-*(ptr2-strip)+*(ptr2-strip-1);
         pre2=tmp2;

         while (act2<-128) act2+=256;
         while (act2>127) act2-=256;

         writebits(DDS_file,act2+(1<<bits2)/2,bits2);
         }

      cnt2=cnt1;
      bits2=bits1;
      }

   if (cnt2!=0)
      {
      writebits(DDS_file,cnt2,DDS_RL);
      writebits(DDS_file,DDS_code(bits2),3);

      while (cnt2-->0)
         {
         tmp2=*ptr2++;
         if (ptr2-strip<=data) act2=tmp2-pre2;
         else act2=tmp2-pre2-*(ptr2-strip)+*(ptr2-strip-1);
         pre2=tmp2;

         while (act2<-128) act2+=256;
         while (act2>127) act2-=256;

         writebits(DDS_file,act2+(1<<bits2)/2,bits2);
         }
      }

   flushbits(DDS_file);
   fclose(DDS_file);

   if (nofree==0) free(data);
   else interleave(data,bytes,skip,DDS_INTERLEAVE);
   }

// read a Differential Data Stream
unsigned char *readDDSfile(const char *filename,size_t *bytes)
   {
   int version=1;

   unsigned int skip,strip;

   unsigned char *data = 0;
   unsigned char *ptr  = 0;

   unsigned int cnt,cnt1,cnt2;
   int bits,act;

   if ((DDS_file=fopen(filename,"rb"))==NULL) return(NULL);

   for (cnt=0; DDS_ID[cnt]!='\0'; cnt++)
      if (fgetc(DDS_file)!=DDS_ID[cnt])
         {
         fclose(DDS_file);
         version=0;
         break;
         }

   if (version==0)
      {
      if ((DDS_file=fopen(filename,"rb"))==NULL) return(NULL);

      for (cnt=0; DDS_ID2[cnt]!='\0'; cnt++)
         if (fgetc(DDS_file)!=DDS_ID2[cnt])
            {
            fclose(DDS_file);
            return(NULL);
            }

      version=2;
      }

   initbuffer();

   skip=readbits(DDS_file,2)+1;
   strip=readbits(DDS_file,16)+1;

   data=NULL;
   cnt=act=0;

   while ((cnt1=readbits(DDS_file,DDS_RL))!=0)
      {
      bits=DDS_decode(readbits(DDS_file,3));

      for (cnt2=0; cnt2<cnt1; cnt2++)
         {
         if (cnt<=strip) act+=readbits(DDS_file,bits)-(1<<bits)/2;
         else act+=*(ptr-strip)-*(ptr-strip-1)+readbits(DDS_file,bits)-(1<<bits)/2;

         while (act<0) act+=256;
         while (act>255) act-=256;

         if (cnt%DDS_BLOCKSIZE==0)
            {
            if (data==NULL)
               {if ((data=(unsigned char *)malloc(DDS_BLOCKSIZE))==NULL) ERRORMSG();}
            else
               if ((data=(unsigned char *)realloc(data,cnt+DDS_BLOCKSIZE))==NULL) ERRORMSG();

            ptr=&data[cnt];
            }

         *ptr++=act;
         cnt++;
         }
      }

   fclose(DDS_file);

   if (cnt==0) return(NULL);

   if ((data=(unsigned char *)realloc(data,cnt))==NULL) ERRORMSG();

   if (version==1) interleave(data,cnt,skip);
   else interleave(data,cnt,skip,DDS_INTERLEAVE);

   *bytes=cnt;

   return(data);
   }

// write a RAW file
void writeRAWfile(const char *filename,unsigned char *data,size_t bytes,int nofree)
   {
   if (bytes<1) ERRORMSG();

   if ((DDS_file=fopen(filename,"wb"))==NULL) ERRORMSG();
   if (fwrite(data,1,bytes,DDS_file)!=bytes) ERRORMSG();

   fclose(DDS_file);

   if (nofree==0) free(data);
   }

// read a RAW file
unsigned char *readRAWfile(const char *filename,size_t *bytes)
   {
   unsigned char *data;
   size_t cnt,blkcnt;

   if ((DDS_file=fopen(filename,"rb"))==NULL) return(NULL);

   data=NULL;
   cnt=0;

   do
      {
      if (data==NULL)
         {if ((data=(unsigned char *)malloc(DDS_BLOCKSIZE))==NULL) ERRORMSG();}
      else
         if ((data=(unsigned char *)realloc(data,cnt+DDS_BLOCKSIZE))==NULL) ERRORMSG();

      blkcnt=fread(&data[cnt],1,DDS_BLOCKSIZE,DDS_file);
      cnt+=blkcnt;
      }
   while (blkcnt==DDS_BLOCKSIZE);

   if (cnt==0)
      {
      free(data);
      return(NULL);
      }

   if ((data=(unsigned char *)realloc(data,cnt))==NULL) ERRORMSG();

   fclose(DDS_file);

   *bytes=cnt;

   return(data);
   }

static void swapshort(unsigned char *ptr,unsigned int size)
   {
   unsigned int i;

   unsigned char tmp;

   for (i=0; i<size; i++)
      {
      tmp=*ptr;
      *ptr=ptr[1]; ptr++;
      *ptr = tmp;  ptr++;
      }
   }

// write an optionally compressed PNM image
void writePNMimage(const char *filename,unsigned const char *image,unsigned int width,unsigned int height,unsigned int components,int dds)
   {
   char str[DDS_MAXSTR];

   unsigned char *data;

   if (width<1 || height<1) ERRORMSG();

   switch (components)
      {
      case 1: snprintf(str,DDS_MAXSTR,"P5\n%d %d\n255\n",width,height); break;
      case 2: snprintf(str,DDS_MAXSTR,"P5\n%d %d\n32767\n",width,height); break;
      case 3: snprintf(str,DDS_MAXSTR,"P6\n%d %d\n255\n",width,height); break;
      default: ERRORMSG();
      }

   if ((data=(unsigned char *)malloc(strlen(str)+width*height*components))==NULL) ERRORMSG();

   memcpy(data,str,strlen(str));
   memcpy(data+strlen(str),image,width*height*components);

   if (components==2) swapshort(data+strlen(str),width*height);

   if (dds!=0) writeDDSfile(filename,data,strlen(str)+width*height*components,components,width);
   else writeRAWfile(filename,data,strlen(str)+width*height*components);
   }

// read a possibly compressed PNM image
unsigned char *readPNMimage(const char *filename,unsigned int *width,unsigned int *height,unsigned int *components)
   {
   const int maxstr=100;

   char str[maxstr];

   unsigned char *data,*ptr1,*ptr2;
   size_t bytes;

   int pnmtype,maxval;
   unsigned char *image;

   if ((data=readDDSfile(filename,&bytes))==NULL)
      if ((data=readRAWfile(filename,&bytes))==NULL) return(NULL);

   if (bytes<4) return(NULL);

   memcpy(str,data,3);
   str[3]='\0';

   if (sscanf(str,"P%1d\n",&pnmtype)!=1) return(NULL);

   ptr1=data+3;
   while (*ptr1=='\n' || *ptr1=='#')
      {
      while (*ptr1=='\n')
         if (++ptr1>=data+bytes) ERRORMSG();
      while (*ptr1=='#')
         if (++ptr1>=data+bytes) ERRORMSG();
         else
            while (*ptr1!='\n')
               if (++ptr1>=data+bytes) ERRORMSG();
      }

   ptr2=ptr1;
   while (*ptr2!='\n' && *ptr2!=' ')
      if (++ptr2>=data+bytes) ERRORMSG();
   if (++ptr2>=data+bytes) ERRORMSG();
   while (*ptr2!='\n' && *ptr2!=' ')
      if (++ptr2>=data+bytes) ERRORMSG();
   if (++ptr2>=data+bytes) ERRORMSG();
   while (*ptr2!='\n' && *ptr2!=' ')
      if (++ptr2>=data+bytes) ERRORMSG();
   if (++ptr2>=data+bytes) ERRORMSG();

   if (ptr2-ptr1>=maxstr) ERRORMSG();
   memcpy(str,ptr1,ptr2-ptr1);
   str[ptr2-ptr1]='\0';

   if (sscanf(str,"%u %u\n%d\n",width,height,&maxval)!=3) ERRORMSG();

   if (*width<1 || *height<1) ERRORMSG();

   if (pnmtype==5 && maxval==255) *components=1;
   else if (pnmtype==5 && (maxval==32767 || maxval==65535)) *components=2;
   else if (pnmtype==6 && maxval==255) *components=3;
   else ERRORMSG();

   if ((image=(unsigned char *)malloc((*width)*(*height)*(*components)))==NULL) ERRORMSG();
   if (data+bytes!=ptr2+(*width)*(*height)*(*components)) ERRORMSG();

   if (*components==2) swapshort(ptr2,(*width)*(*height));

   memcpy(image,ptr2,(*width)*(*height)*(*components));
   free(data);

   return(image);
   }

// write a compressed PVM volume
void writePVMvolume(const char *filename,unsigned const char *volume,
                    unsigned int width,unsigned int height,unsigned int depth,unsigned int components,
                    float scalex,float scaley,float scalez,
                    unsigned const char *description,
                    unsigned const char *courtesy,
                    unsigned const char *parameter,
                    unsigned const char *comment)
   {
   char str[DDS_MAXSTR];

   unsigned char *data;

   size_t len1=1,len2=1,len3=1,len4=1;

   if (width<1 || height<1 || depth<1 || components<1) ERRORMSG();

   if (description==NULL && courtesy==NULL && parameter==NULL && comment==NULL)
      if (scalex==1.0f && scaley==1.0f && scalez==1.0f)
         snprintf(str,DDS_MAXSTR,"PVM\n%d %d %d\n%d\n",width,height,depth,components);
      else
         snprintf(str,DDS_MAXSTR,"PVM2\n%d %d %d\n%g %g %g\n%d\n",width,height,depth,scalex,scaley,scalez,components);
   else
      snprintf(str,DDS_MAXSTR,"PVM3\n%d %d %d\n%g %g %g\n%d\n",width,height,depth,scalex,scaley,scalez,components);

   if (description==NULL && courtesy==NULL && parameter==NULL && comment==NULL)
      {
      if ((data=(unsigned char *)malloc(strlen(str)+width*height*depth*components))==NULL) ERRORMSG();

      memcpy(data,str,strlen(str));
      memcpy(data+strlen(str),volume,width*height*depth*components);

      writeDDSfile(filename,data,strlen(str)+width*height*depth*components,components,width);
      }
   else
      {
      if (description!=NULL) len1=strlen((char *)description)+1;
      if (courtesy!=NULL) len2=strlen((char *)courtesy)+1;
      if (parameter!=NULL) len3=strlen((char *)parameter)+1;
      if (comment!=NULL) len4=strlen((char *)comment)+1;

      if ((data=(unsigned char *)malloc(strlen(str)+width*height*depth*components+len1+len2+len3+len4))==NULL) ERRORMSG();

      memcpy(data,str,strlen(str));
      memcpy(data+strlen(str),volume,width*height*depth*components);

      if (description==NULL) *(data+strlen(str)+width*height*depth*components)='\0';
      else memcpy(data+strlen(str)+width*height*depth*components,description,len1);

      if (courtesy==NULL) *(data+strlen(str)+width*height*depth*components+len1)='\0';
      else memcpy(data+strlen(str)+width*height*depth*components+len1,courtesy,len2);

      if (parameter==NULL) *(data+strlen(str)+width*height*depth*components+len1+len2)='\0';
      else memcpy(data+strlen(str)+width*height*depth*components+len1+len2,parameter,len3);

      if (comment==NULL) *(data+strlen(str)+width*height*depth*components+len1+len2+len3)='\0';
      else memcpy(data+strlen(str)+width*height*depth*components+len1+len2+len3,comment,len4);

      writeDDSfile(filename,data,strlen(str)+width*height*depth*components+len1+len2+len3+len4,components,width);
      }
   }

// read a compressed PVM volume
unsigned char *readPVMvolume(const char *filename,
                             unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *components,
                             float *scalex,float *scaley,float *scalez,
                             unsigned char **description,
                             unsigned char **courtesy,
                             unsigned char **parameter,
                             unsigned char **comment)
   {
   unsigned char *data,*ptr;
   size_t bytes;
   unsigned int numc;

   int version=1;

   unsigned char *volume;

   float sx=1.0f,sy=1.0f,sz=1.0f;

   size_t len1=0,len2=0,len3=0,len4=0;

   if ((data=readDDSfile(filename,&bytes))==NULL) return(NULL);
   if (bytes<5) return(NULL);

   if ((data=(unsigned char *)realloc(data,bytes+1))==NULL) ERRORMSG();
   data[bytes]='\0';

   if (strncmp((char *)data,"PVM\n",4)!=0)
      {
      if (strncmp((char *)data,"PVM2\n",5)==0) version=2;
      else if (strncmp((char *)data,"PVM3\n",5)==0) version=3;
      else { free(data); return(NULL); }

      if (sscanf((char *)&data[5],"%u %u %u\n%g %g %g\n",width,height,depth,&sx,&sy,&sz)!=6) ERRORMSG();
      if (*width<1 || *height<1 || *depth<1 || sx<=0.0f || sy<=0.0f || sz<=0.0f) ERRORMSG();
      ptr=(unsigned char *)strchr((char *)&data[5],'\n')+1;
      }
   else
      {
      if (sscanf((char *)&data[4],"%u %u %u\n",width,height,depth)!=3) ERRORMSG();
      if (*width<1 || *height<1 || *depth<1) ERRORMSG();
      ptr=&data[4];
      }

   if (scalex!=NULL && scaley!=NULL && scalez!=NULL)
      {
      *scalex=sx;
      *scaley=sy;
      *scalez=sz;
      }

   ptr=(unsigned char *)strchr((char *)ptr,'\n')+1;
   if (sscanf((char *)ptr,"%u\n",&numc)!=1) ERRORMSG();
   if (numc<1) ERRORMSG();

   if (components!=NULL) *components=numc;
   else if (numc!=1) ERRORMSG();

   ptr=(unsigned char *)strchr((char *)ptr,'\n')+1;
   if (version==3) len1=strlen((char *)(ptr+(*width)*(*height)*(*depth)*numc))+1;
   if (version==3) len2=strlen((char *)(ptr+(*width)*(*height)*(*depth)*numc+len1))+1;
   if (version==3) len3=strlen((char *)(ptr+(*width)*(*height)*(*depth)*numc+len1+len2))+1;
   if (version==3) len4=strlen((char *)(ptr+(*width)*(*height)*(*depth)*numc+len1+len2+len3))+1;
   if ((volume=(unsigned char *)malloc((*width)*(*height)*(*depth)*numc+len1+len2+len3+len4))==NULL) ERRORMSG();
   if (data+bytes!=ptr+(*width)*(*height)*(*depth)*numc+len1+len2+len3+len4) ERRORMSG();

   memcpy(volume,ptr,(*width)*(*height)*(*depth)*numc+len1+len2+len3+len4);
   free(data);

   if (description!=NULL)
      {
      if (len1>1) *description=volume+(*width)*(*height)*(*depth)*numc;
      else *description=NULL;
      }

   if (courtesy!=NULL)
      {
      if (len2>1) *courtesy=volume+(*width)*(*height)*(*depth)*numc+len1;
      else *courtesy=NULL;
      }

   if (parameter!=NULL)
      {
      if (len3>1) *parameter=volume+(*width)*(*height)*(*depth)*numc+len1+len2;
      else *parameter=NULL;
      }

   if (comment!=NULL)
      {
      if (len4>1) *comment=volume+(*width)*(*height)*(*depth)*numc+len1+len2+len3;
      else *comment=NULL;
      }

   return(volume);
   }

// simple checksum algorithm
unsigned int checksum(unsigned char *data, unsigned int bytes)
   {
   const unsigned int prime=271;

   unsigned int i;

   unsigned char *ptr,value;

   unsigned int sum,cipher;

   sum=0;
   cipher=1;

   for (ptr=data,i=0; i<bytes; i++)
      {
      value=*ptr++;
      cipher=prime*cipher+value;
      sum+=cipher*value;
      }

   return(sum);
   }

// swap the hi and lo byte of 16 bit data
void swapbytes(unsigned char *data,unsigned int bytes)
   {
   unsigned int i;
   unsigned char *ptr,tmp;

   for (ptr=data,i=0; i<bytes/2; i++,ptr+=2)
      {
      tmp=*ptr;
      *ptr=*(ptr+1);
      *(ptr+1)=tmp;
      }
   }

// convert from signed short to unsigned short
void convbytes(unsigned char *data,unsigned int bytes)
   {
   unsigned int i;
   unsigned char *ptr;
   int v,vmin;

   for (vmin=32767,ptr=data,i=0; i<bytes/2; i++,ptr+=2)
      {
      v=256*(*ptr)+*(ptr+1);
      if (v>32767) v=v-65536;
      if (v<vmin) vmin=v;
      }

   for (ptr=data,i=0; i<bytes/2; i++,ptr+=2)
      {
      v=256*(*ptr)+*(ptr+1);
      if (v>32767) v=v-65536;

      *ptr=(v-vmin)/256;
      *(ptr+1)=(v-vmin)%256;
      }
   }

// convert from float to unsigned short
void convfloat(unsigned char *data,unsigned int bytes)
   {
   unsigned int i;
   unsigned char *ptr;
   float v,vmax;

   for (vmax=1.0f,ptr=data,i=0; i<bytes/4; i++,ptr+=4)
      {
      DDS_swapuint((unsigned int *)ptr);

      v = static_cast<float>(fabs(*((float *)ptr)));
      if (v>vmax) vmax=v;
      }

   for (ptr=data,i=0; i<bytes/4; i++,ptr+=4)
      {
      v=static_cast<float>(fabs(*((float *)ptr)))/vmax;

      data[2*i]=ftrc(65535.0f*v+0.5f)/256;
      data[2*i+1]=ftrc(65535.0f*v+0.5f)%256;
      }
   }

// helper functions for quantize:

inline int DDS_get(unsigned short int *volume,
                   unsigned int width,unsigned int height,
                   unsigned int /*depth*/,
                   unsigned int i,unsigned int j,unsigned int k)
   {return(volume[i+(j+k*height)*width]);}

inline double DDS_getgrad(unsigned short int *volume,
                          unsigned int width,unsigned int height,unsigned int depth,
                          unsigned int i,unsigned int j,unsigned int k)
   {
   double gx,gy,gz;

   if (i>0)
      if (i<width-1) gx=(DDS_get(volume,width,height,depth,i+1,j,k)-DDS_get(volume,width,height,depth,i-1,j,k))/2.0;
      else gx=DDS_get(volume,width,height,depth,i,j,k)-DDS_get(volume,width,height,depth,i-1,j,k);
   else gx=DDS_get(volume,width,height,depth,i+1,j,k)-DDS_get(volume,width,height,depth,i,j,k);

   if (j>0)
      if (j<height-1) gy=(DDS_get(volume,width,height,depth,i,j+1,k)-DDS_get(volume,width,height,depth,i,j-1,k))/2.0;
      else gy=DDS_get(volume,width,height,depth,i,j,k)-DDS_get(volume,width,height,depth,i,j-1,k);
   else gy=DDS_get(volume,width,height,depth,i,j+1,k)-DDS_get(volume,width,height,depth,i,j,k);

   if (k>0)
      if (k<depth-1) gz=(DDS_get(volume,width,height,depth,i,j,k+1)-DDS_get(volume,width,height,depth,i,j,k-1))/2.0;
      else gz=DDS_get(volume,width,height,depth,i,j,k)-DDS_get(volume,width,height,depth,i,j,k-1);
   else gz=DDS_get(volume,width,height,depth,i,j,k+1)-DDS_get(volume,width,height,depth,i,j,k);

   return(sqrt(gx*gx+gy*gy+gz*gz));
   }

// quantize 16 bit volume to 8 bit using a non-linear mapping
unsigned char *quantize(unsigned char *volume,
                        unsigned int width,unsigned int height,unsigned int depth,
                        BOOLINT nofree,
                        BOOLINT linear,
                        BOOLINT verbose)
   {
   unsigned int i,j,k;

   unsigned char *volume2;
   unsigned short int *volume3;

   int v,vmin,vmax;

   double *err,eint;

   err=new double[65536];

   BOOLINT done;

   if ((volume3=(unsigned short int*)malloc(width*height*depth*sizeof(unsigned short int)))==NULL) ERRORMSG();

   vmin=vmax=256*volume[0]+volume[1];

   for (k=0; k<depth; k++)
      for (j=0; j<height; j++)
         for (i=0; i<width; i++)
            {
            v=256*volume[2*(i+(j+k*height)*width)]+volume[2*(i+(j+k*height)*width)+1];
            volume3[i+(j+k*height)*width]=v;

            if (v<vmin) vmin=v;
            else if (v>vmax) vmax=v;
            }

   if (!nofree) free(volume);

   if (verbose)
      printf("16 bit volume has scalar range=[%d,%d]\n",vmin,vmax);

   if (linear)
      for (i=0; i<65536; i++) err[i]=255*(double)i/vmax;
   else
      {
      for (i=0; i<65536; i++) err[i]=0.0;

      for (k=0; k<depth; k++)
         for (j=0; j<height; j++)
            for (i=0; i<width; i++)
               err[DDS_get(volume3,width,height,depth,i,j,k)]+=sqrt(DDS_getgrad(volume3,width,height,depth,i,j,k));

      for (i=0; i<65536; i++) err[i]=pow(err[i],1.0/3);

      err[vmin]=err[vmax]=0.0;

      for (k=0; k<256; k++)
         {
         for (eint=0.0,i=0; i<65536; i++) eint+=err[i];

         done=TRUE;

         for (i=0; i<65536; i++)
            if (err[i]>eint/256)
               {
               err[i]=eint/256;
               done=FALSE;
               }

         if (done) break;
         }

      for (i=1; i<65536; i++) err[i]+=err[i-1];

      if (err[65535]>0.0f)
         for (i=0; i<65536; i++) err[i]*=255.0f/err[65535];
      }

   if ((volume2=(unsigned char *)malloc(width*height*depth))==NULL) ERRORMSG();

   for (k=0; k<depth; k++)
      for (j=0; j<height; j++)
         for (i=0; i<width; i++)
            volume2[i+(j+k*height)*width]=(int)(err[DDS_get(volume3,width,height,depth,i,j,k)]+0.5);

   delete [] err;
   free(volume3);

   if (verbose)
      printf("quantized volume to 8 bit\n");

   return(volume2);
   }