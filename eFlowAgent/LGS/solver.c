#include <ped.h>

extern SOLV solver;
extern int NK;

REAL lgs_solver(REAL time, REAL *A, REAL *x, REAL *y, int flag, void (*b)(REAL , REAL *, int ), void (*amul_bd)(REAL *, REAL *, REAL * ))
{
  REAL RES=1000;

  switch (flag){
  case 1:
    RES = PCG(time,A,x,y,b,amul_bd,solver.precon,solver.akrit);

    if(0){
      REAL *bla, gaga;
      bla = (REAL *)calloc(NK , sizeof(REAL ));
      amul(A,x,bla);
      field_cp(bla,1.,bla,-1.,y,0.,NK);
      b(time,bla,0);
      gaga = sqrt(scalpr(bla,bla,NK));
      printf("bla........................................ Err: %6.2e\n",gaga);
    }
    break;
  case 2:
    RES = PBICGSTAB(time,A,x,y,b,amul_bd,solver.precon,solver.akrit);

    if(0){
      REAL *bla, gaga;
      bla = (REAL *)calloc(NK , sizeof(REAL ));
      amul(A,x,bla);
      b(time,bla,0);
      field_cp(bla,1.,bla,-1.,y,0.,NK);
      gaga = sqrt(scalpr(bla,bla,NK));
      printf("........................................... Err: %6.2e\n",gaga);
    }

    break;
  default:

    ErrorMessage("No existing solver chosen","lgs_solver");

    break;
  }


  return RES;
}
