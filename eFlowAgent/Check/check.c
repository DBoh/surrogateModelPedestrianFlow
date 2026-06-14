#include <ped.h>

extern int NT, NK;
extern int *nach, bandwidth;
extern REAL *XY;
extern char femlog[200];
extern GN *node;
extern GE *tri;

void check_symmetrie(REAL *A)
{
  int i, j, l, Err=0;
  int ii, jj, kk, ll;
  REAL a1, a2, eps=1.E-08;

  for(i=0 ; i<NK ; i++){
    for(l=node[i].pointmat+1 ; l<node[i+1].pointmat ; l++){
      j = nach[l];
      if(j>i){
  a1 = A[l];
  for(ll=node[j].pointmat+1 ; ll<node[j+1].pointmat ; ll++){
    jj = nach[ll];
    if(jj==i){
      a2 = A[ll];
      if(fabs(a1-a2)>eps){
        printf("Error in check_symmetrie!\n");
        Err ++;
      }
    }
  }

      }
    }
  }

  printf("\n%d Errors found in check_symmetrie\n\n",Err);

}


void check_matrix(REAL *A)
{
  int i, j;
  REAL *u, x, y, *v, *w, auu=0.;

  //  check_symmetrie(A);

  u = calloc(NK , sizeof(REAL));
  v = calloc(NK , sizeof(REAL));
  w = calloc(NK , sizeof(REAL));


  for(i=0 ; i<NK ; i++){
    x = XY[2*i];
    y = XY[2*i+1];
    u[i] = 1.;//3*x+y;
    v[i] = 3*x+y;
    /*
      M+S+K mit v=1:
      u=3x+y;
      Auu = 232/3 + 40 + 64
          ~ 77.33 + 40 + 64 = 181.33
    */
  }

  amul(A,u,w);
  auu = scalpr(w,v,NK);

  printf("int = %f\n",auu);

}

void check_rhs(REAL *f[2])
{
  int i;
  REAL *f1, *f2, *v, fv1, fv2;

  v  = calloc(NK , sizeof(REAL));
  f1 = calloc(NK , sizeof(REAL));
  f2 = calloc(NK , sizeof(REAL));

  for(i=0 ; i<NK ; i++){
    f1[i] = f[0][i];
    f2[i] = f[1][i];
    v[i] = XY[2*i];
  }

  fv1 = scalpr(f1,v,NK);
  fv2 = scalpr(f2,v,NK);

  printf("rsh_check: %f %f\n",fv1,fv2);

}

void print_grid()
{
  int i, j, k;

  printf("Anzahl der Dreiecke: %d\n",NT);
  printf("Anzahl der Knoten:   %d\n",NK);


  printf("XY-Koordinaten:      \n");
  for(i=0 ; i<NK ; i++)
    printf("X_%d = %f %f\n",i,XY[2*i],XY[2*i+1]);


  printf("Globale Nummerierung:\n");
  for(i=0 ; i<NT ; i++)
    printf("%d: %d %d %d\n",i,tri[i].VNR[0],tri[i].VNR[1],tri[i].VNR[2]);


  printf("nachbarschaften:\n");
  for(i=0 ; i<NT ; i++)
    printf("%d: %d %d %d\n",i,tri[i].NEIGH[0],tri[i].NEIGH[1],tri[i].NEIGH[2]);



}


void printlog(char *s, int flag)
{
  FILE *fp;

  switch (flag){
  case 0:
    fp = fopen(femlog,"w");
    fprintf(fp,"%s \n",s);
    fclose(fp);
    break;
  default:
    fp = fopen(femlog,"a");
    fprintf(fp,"%s \n",s);
    fclose(fp);
    break;
  }

}


