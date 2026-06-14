#include <ped.h>

extern int NK, bandwidth;
extern REAL *XY;

void PrintSysMatrix(char *fname, REAL *A)
{
  REAL *v, *w;
  int i, j;
  FILE *fp;

  v = (REAL *)calloc(NK , sizeof(REAL));
  w = (REAL *)calloc(NK , sizeof(REAL));

  fp = fopen(fname,"w");

  for(i=0 ; i<NK ; i++){
    for(j=0 ; j<NK ; j++)
      v[j] = 0.;
    v[i] = 1.;

    amul(A,v,w);

    for(j=0 ; j<NK ; j++)
      fprintf(fp,"%.4e ",w[j]);
    //      fprintf(fp,"%f ",w[j]);
    fprintf(fp,"\n");
  }

  fclose(fp);

  free(v); free(w);
}


void printmatrix(char *fname, REAL A[6][6], int N)
{
  int i, j;
  FILE *fp;

  fp = fopen(fname,"w");

  for(i=0 ; i<N ; i++){
    for(j=0 ; j<N ; j++)
      fprintf(fp,"%f ",A[i][j]);
    fprintf(fp,"\n");
  }

  fclose(fp);

}


void printvector(char *fname, REAL *v)
{
  int i;
  FILE *fp;

  fp = fopen(fname,"w");

  for(i=0 ; i<NK ; i++)
    fprintf(fp,"%f ",v[i]);

  fclose(fp);

}


void printXY(char *fname)
{
  int i;
  FILE *fp;

  fp = fopen(fname,"w");

  for(i=0 ; i<NK ; i++)
    fprintf(fp,"%f %f \n",XY[2*i],XY[2*i+1]);

  fclose(fp);

}


