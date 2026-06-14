/*!
  \file tools.c
  \brief some unclassified routines
*/
#include <ped.h>
extern char femlog[200];
extern Monitoring MONI;

void Message(char *bla1, char bla2, char *bla3)
{
#if WEB
  sprintf(MONI.Message,"Error in %s: %s",bla3,bla1);
#else
  int l1, l2, ll, i, maxl, cmd, add=0;
  char *msg;

  cmd = strcmp(bla1,"\n");
  if(!cmd) add=1;

  maxl = 70+add;
  msg  = (char *)malloc((maxl+1) * sizeof(char ));

  l1=strlen(bla1);
  l2=strlen(bla3);
  ll = maxl-(l2+l1);


  for(i=0 ; i<maxl ; i++){
    if(i<l1)
      msg[i] = bla1[i];
    else if(i<ll+l1)
      msg[i] = bla2;
    else
      msg[i] = bla3[i-(ll+l1)];
  }
  msg[maxl] = '\0';

  printf("%s\n",msg);

  // logfile save
  sprintf(msg,"Error in %s: %s",bla3,bla1);
  WriteLog(msg,"a");
  
  free(msg);
#endif
}

/*! \brief Erzeugt eine Fehlermeldung und bricht das Programm ab*/
void ErrorMessage(char *bla, char *function)
{
#if WEB
  sprintf(MONI.Message,"Error in %s: ",function,bla);
#else
  char *mesg;
  mesg = (char *)malloc(61*sizeof(char ));

  sprintf(mesg,"Error in %s",function);
  Message(mesg,'_',bla);

  free(mesg);
  exit(0);
#endif
}

void add_numer_to_string(char *bla, int num, int n)
{
  char gugu[100];

  sprintf(gugu,"_%07d",num);
  strcat(bla,gugu);
    
}

/*!
  \brief p-norm of a vector
  \detail
  \f[\|v\|_p=\left(\sum_{i=0}^{N-1}v_i^p\right)^{\frac{1}{p}}\f]
*/
REAL norm(REAL *v, int N, REAL p)
{
  int i;
  REAL out=0.;

  for(i=0 ; i<N ; i++)
    out += pow(v[i],p);
  out = pow(out,1./p);

  return out;
}

/*!
  \brief p-norm of a vector
  \detail
  \f[\|v\|_p=\left(\sum_{i=0}^{N-1}v_i^p\right)^{\frac{1}{p}}\f]
*/
REAL norm2(REAL *v, int N)
{
  int i;
  REAL out=0.;

  for(i=0 ; i<N ; i++)
    out += v[i]*v[i];
  out = sqrt(out);

  return out;
}


/*!
  \brief prints a field to STDOUT

  \detail

  <table>
  <tr><td>
  intput:</td><td>f=field of size MxN</td>
  </tr>
  <tr>
    <td></td><td>write a subject bla</td>
  </tr>
  <tr>
    <td></td><td>if flag=0: fields with more than 10 entries are not printed</td>
  </tr>
  </table>
*/
void print_field(REAL *f, int M, int N, char *bla)
{
  int i, j, ij;
  FILE *fp;

  fp = fopen(bla,"w");

  for(i=0 ; i<M ; i++){
    for(j=0 ; j<N ; j++){
      ij = i*N+j;
      fprintf(fp,"%6.2f ",f[ij]);
    }
    fprintf(fp,"\n");
  }
  fprintf(fp,"\n");
  fclose(fp);
}


void FieldCpInt(int *d, int alpha, int *a, int gamma, int N)
{
  int i;

  for(i=0 ; i<N ; i++)
    d[i] = alpha*a[i]+gamma;

}

void FieldCpREAL(REAL *d, REAL alpha, REAL *a, REAL gamma, int N)
{
  int i;

  for(i=0 ; i<N ; i++)
    d[i] = alpha*a[i]+gamma;

}

//! d=alpha*a+gamma
void FieldCp(REAL *d, REAL alpha, REAL *a, REAL gamma, int N)
{
  int i;

  for(i=0 ; i<N ; i++)
    d[i] = alpha*a[i]+gamma;

}

//! d=alpha*a:b+gamma (kpt-vise mult)
void FieldMl(REAL *d, REAL alpha, REAL *a, REAL *b, REAL gamma, int N)
{
  int i;

  for(i=0 ; i<N ; i++)
    d[i] = alpha*a[i]*b[i]+gamma;

}

//! d = alpha*a+beta*b+gamma
void field_cp(REAL *d, REAL alpha, REAL *a, REAL beta, REAL *b, REAL gamma, int N)
{
  int i;

  for(i=0 ; i<N ; i++)
    d[i] = alpha*a[i]+beta*b[i]+gamma;

}

int FieldMin_int(int *f, int N)
{
  int i;
  int fmin=f[0];

  for(i=1 ; i<N ; i++)
    if(f[i]<fmin) fmin=f[i];

  return fmin;
}

int FieldMax_int(int *f, int N)
{
  int i;
  int fmax=f[0];

  for(i=1 ; i<N ; i++)
    if(f[i]>fmax) fmax=f[i];

  return fmax;
}

REAL FieldMin(REAL *f, int N)
{
  int i;
  REAL fmin=f[0];

  for(i=1 ; i<N ; i++)
    if(f[i]<fmin) fmin=f[i];

  return fmin;
}

REAL FieldMax(REAL *f, int N)
{
  int i;
  REAL fmax=f[0];

  for(i=1 ; i<N ; i++)
    if(f[i]>fmax) fmax=f[i];

  return fmax;
}


void UpdateFieldsMinMax(REAL *f, int N, REAL *fmin, REAL *fmax) {

  REAL min = FieldMin(f, N);
  REAL max = FieldMax(f, N);

  if (min < *fmin) { *fmin = min; }
  if (max > *fmax) { *fmax = max; }
}

void PrintFieldMinMax(REAL time, REAL *f, int N, REAL *fmin, REAL *fmax, char *name, char *type)
{
  REAL min, max;
  char fname[100];
  FILE *fp;

  min = FieldMin(f,N);
  max = FieldMax(f,N);

  if(min<*fmin) *fmin=min;
  if(max>*fmax) *fmax=max;

  sprintf(fname,"00Output/%sminmax.dat",name);
  fp=fopen(fname,type);
  fprintf(fp,"%f %f %f %f %f\n",time,*fmin,min,max,*fmax);
  fclose(fp);
}


