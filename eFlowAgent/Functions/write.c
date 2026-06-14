/*!
  \file write.c
  \brief writes an reads computed data
*/
#include  <ped.h>

extern GE *tri;
extern GN *node;
extern int NT, NK;
extern REAL *XY;
extern VIDEO video;
extern DAT PED;
extern TRI refine;

int FLOW=0;


void LinePlot(REAL ax, REAL ay, REAL bx, REAL by, REAL *u, char *name, int loop)
{
  int i, DIM, tri=0;
  REAL h, len, xx, yy;
  char fname[100];
  FILE *fp;

  h = refine.gridwidth/100;
  len = sqrt(pow(ax-bx,2.)+pow(ay-by,2.));
  DIM = (int)(len/h)+1;
  h = 1.0/(REAL)(DIM-1);

  sprintf(fname,"GnuPlot/%s",name);
  add_numer_to_string(fname,loop,7);

  strcat(fname,".dat");
  fp = fopen(fname,"w");

  for(i=0 ; i<DIM ; i++){
    xx = ax+i*h*(bx-ax);
    yy = ay+i*h*(by-ay);
    tri = search_tri(tri,xx,yy,0);
    if(tri>=0)
      fprintf(fp,"%f %f %f\n",xx,yy,uh(tri,xx,yy,u));
    else
      tri=0;
  }
  fclose(fp);

}


//! Write Date for the glorious GUI
void WriteRhoVel(REAL *rho, REAL *v[2],char *fname,int loop)
{
  FILE *fp;
  char name[100];
  int k, l, j;

  sprintf(name,"GnuPlot/%s",fname);
  add_numer_to_string(name,loop,7);
  strcat(name,"%s.dat");

  fp = fopen(name,"w");

  for(j=0 ; j<NK ; j++)
    fprintf(fp,"%f %f %f %f %f\n",XY[2*j],XY[2*j+1],rho[j],v[0][j],v[1][j]);

  fclose(fp);

  #if GUI
  printf("%s\n",name);
  fflush(stdout);
  #endif

}

void ReadRhoVel(REAL *rho, REAL *v[2],char *fname,int loop)
{
  FILE *fp;
  REAL bla1, bla2;
  char name[100], linebuf[100];
  int k, l, j;

  sprintf(name,"GnuPlot/%s",fname);
  add_numer_to_string(name,loop,7);
  strcat(name,".dat");

  fp = fopen(name,"r");

  if(fp){
    for(j=0 ; j<NK ; j++){
      fgets(linebuf, sizeof(linebuf), fp);
      sscanf(linebuf,"%lf %lf %lf %lf %lf\n",&bla1,&bla2,&rho[j],&v[0][j],&v[1][j]);
    }

    fclose(fp);
  }
}


void WriteTrackPoints(REAL *rho, char *fname, int loop)
{
  FILE *fp;
  char name[100];
  int k=0, kk, l, j, Num=video.TrackPersonMax, icount=0;

  sprintf(name,"Octave/%s",fname);
  add_numer_to_string(name,loop,7);
  strcat(name,".dat");

  for(j=0 ; j<Num ; j++)
    icount += video.TrackInside[j];


  fp = fopen(name,"w");
  if(icount){

    for(j=0 ; j<Num ; j++){
      if(video.TrackInside[j]){
	kk = search_tri(k,video.TrackXYPosition[0][j],video.TrackXYPosition[1][j],0);
	if(kk>=0){
	  k = kk;
	  video.Rhoh[j] = uh(k,video.TrackXYPosition[0][j],video.TrackXYPosition[1][j],rho);
	}
	fprintf(fp,"%d %f %f %f %d\n",j,video.TrackXYPosition[0][j],video.TrackXYPosition[1][j],video.Rhoh[j],video.SIRmode[j]);
      }
    }
  }
  fclose(fp);
}


void ReadTrackPoints(REAL *rho, char *fname, int loop)
{
  FILE *fp;
  char name[100], linebuf[100], msg[100];
  int k, l, j, Num=video.TrackPersonMax;

  sprintf(name,"GnuPlot/TrackPoints/%s",fname);
  add_numer_to_string(name,loop,7);
  strcat(name,".dat");

  fp = fopen(name,"r");
  if(fp) {
    for(j=0 ; j<Num ; j++){
      fgets(linebuf,sizeof(linebuf),fp);
      sscanf(linebuf,"%d %lf %lf %lf \n",&video.TrackInside[j],&video.TrackXYPosition[0][j],&video.TrackXYPosition[1][j],&video.Rhoh[j]);
    }

    fclose(fp);

    #if GUI
    printf("%s\n",name);
    #endif

  }

}


void WriteVRTrack(REAL *rho, REAL *v[2],char *fname,REAL time, char *type)
{
  FILE *fp;
  char name[100];
  REAL rhoh, velh, vel[2];
  int k=0, l, j, icount=0, Num=video.TrackPersonMax;
  int kk;

  sprintf(name,"GUIData/%s.dat",fname);

  fp = fopen(name,type);

  for(j=0 ; j<Num ; j++)
    icount += video.TrackInside[j];


  if(icount){
    for(j=0 ; j<Num ; j++){
      if(video.TrackInside[j]){
  kk = search_tri(k,video.TrackXYPosition[0][j],video.TrackXYPosition[1][j],0);
  if(kk>=0){
    k = kk;
    rhoh = uh(k,video.TrackXYPosition[0][j],video.TrackXYPosition[1][j],rho);
    rhoh = min_REAL(max_REAL(0.,rhoh),1.);
    velh = FD_Value(rhoh,0,video.TrackXYPosition[0][j],video.TrackXYPosition[1][j],1);
  }
  fprintf(fp,"%d %d %f %f %f %f %f\n",j,1,time,video.TrackXYPosition[0][j],video.TrackXYPosition[1][j],rhoh,velh);
      } else
  fprintf(fp,"%d 0 %f 0. 0. 0. 0.\n",j,time);
    }
  }
  fclose(fp);

}

void WriteLog(char *s, char *type)
{
  extern char femlog[200];
  FILE *fp;
  int i;
  
  fp = fopen(femlog,type);
  fprintf(fp,"%s\n",s);
  for(i=0 ; i<60; i++)
    fprintf(fp,".");
  fprintf(fp,"\n");
  fclose(fp);

  
}


