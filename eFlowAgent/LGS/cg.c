/*!
  \file cg.c
  \brief conjugate gradient algorithm
*/
#include <ped.h>

extern int NK, bandwidth;
extern GN *node;

/* http://www.mathepedia.de/CG-Verfahren.html */

REAL PCG(REAL time, REAL *A,REAL *x,REAL *b, void (*bound)(REAL , REAL *, int ), void (*amul_bd)(REAL *, REAL *, REAL *), int pcflag, REAL akrit)
{
  int k, i;
  REAL d0, d1, alpha, beta;

  REAL res = 0; // TODO INTIALISATION 0 or NaN ?

  REAL *C, *ad, *g, *h, *ax, *d;
  C  = (REAL *)calloc(NK , sizeof(REAL ));
  ad = (REAL *)calloc(NK , sizeof(REAL ));
  ax = (REAL *)calloc(NK , sizeof(REAL ));
  g  = (REAL *)calloc(NK , sizeof(REAL ));
  h  = (REAL *)calloc(NK , sizeof(REAL ));
  d  = (REAL *)calloc(NK , sizeof(REAL ));

  FieldCp(C,0.,C,1.,NK);

  if(pcflag){
    for(i=0 ; i<NK ; i++)
      C[i] = 1./A[node[i].pointmat];
  }

  bound(time,b,1);
  bound(time,x,1);

  amul_bd(A,x,ax);

  field_cp(g,1.,b,-1.,ax,0.,NK);      //  g=b-ax;
  FieldMl(h,1.,C,g,0.,NK);            //  h=C*g;
  FieldCp(d,-1.,h,0.,NK);             //  d=-h;

  for(k=0 ; k<NK ; k++) {

    amul_bd(A,d,ad);

    d1 = scalpr(g,h,NK);              //d1 = g'*h;
    d0 = scalpr(d,ad,NK);
    alpha = d1/d0;                    //d1/(d'*ad);

    field_cp(x,1.,x,-alpha,d,0.,NK);  // x = x - alpha*d;
    field_cp(g,1.,g,alpha,ad,0.,NK);  // g = g + alpha*ad;
    FieldMl(h,1.,C,g,0.,NK);          // h = C*g;

    d0 = scalpr(g,h,NK);              // d0 = g'*h;
    beta = d0/d1;

    field_cp(d,-1.,h,beta,d,0.,NK);   // d = -h + beta*d;

    res = sqrt(scalpr(g,g,NK));

    if(MSG>2)
      printf("%d-te Iteration mit Residuum %.2e\n",k,res);

    if(res<akrit){
      if(MSG>1){
        printf("NumIt = %d \n",k);// res = %6.2e, |D| = %6.2e ",k,res,Dmax);
      }

      break;
    }

  }


  free( C  );
  free( ad );
  free( ax );
  free( g  );
  free( h  );
  free( d  );

  return res;
}


/* Yousef Saad, p 219 */
REAL PBICGSTAB(REAL time, REAL *A,REAL *x,REAL *b, void (*bound)(REAL , REAL *, int ), void (*amul_bd)(REAL *, REAL *, REAL *), int pcflag, REAL akrit)
{
  int k, i;
  REAL Dmax;
  REAL d1, d0, alpha, beta, omega;
  REAL *C, *ax, *r, *dr0, *dp, *ds, *dr, *as, *das, *dap, *ap;

  REAL res = 0; // TODO INTIALISATION 0 or NaN ?


  C    = (REAL *)calloc(NK , sizeof(REAL ));
  ax   = (REAL *)calloc(NK , sizeof(REAL ));
  r    = (REAL *)calloc(NK , sizeof(REAL ));
  dr   = (REAL *)calloc(NK , sizeof(REAL ));
  dr0  = (REAL *)calloc(NK , sizeof(REAL ));
  dp   = (REAL *)calloc(NK , sizeof(REAL ));
  as   = (REAL *)calloc(NK , sizeof(REAL ));
  das  = (REAL *)calloc(NK , sizeof(REAL ));
  dap  = (REAL *)calloc(NK , sizeof(REAL ));
  ap   = (REAL *)calloc(NK , sizeof(REAL ));
  ds   = (REAL *)calloc(NK , sizeof(REAL ));


  FieldCp(C,0.,C,1.,NK);

  if(pcflag){
    for(i=0 ; i<NK ; i++)
      C[i] = 1./A[node[i].pointmat];
  }
  Dmax = fabs(FieldMax(C,NK));

      
  bound(time,b,1);
  bound(time,x,1);

  //  amul(A,x,ax);
  amul_bd(A,x,ax);

  field_cp(r,1.,b,-1.,ax,0.,NK);             // r  = b-ax;

  res = norm2(r,NK);
  if(res>=akrit)
    {
      
      FieldMl(dr,1.,C,r,0.,NK);                  // dr  = C(b-ax)
      FieldCp(dr0,1.,dr,0.,NK);                  // dr0 = dr; % arbitrary
      FieldCp(dp,1.,dr,0.,NK);                   // dp  = dr;
      
      for(k=1 ; k<NK ; k++){
	
	//    amul(A,dp,ap);
	amul_bd(A,dp,ap);
	FieldMl(dap,1.,C,ap,0.,NK);
	
	d1    = scalpr(dr,dr0,NK);
	alpha = d1/scalpr(dap,dr0,NK);
	
	field_cp(ds,1.,dr,-alpha,dap,0.,NK);
	
	//    amul(A,ds,as);
	amul_bd(A,ds,as);
	FieldMl(das,1.,C,as,0.,NK);
	
	omega = scalpr(das,ds,NK)/scalpr(das,das,NK);
	
	field_cp(x,1.,x,alpha,dp,0.,NK);
	field_cp(x,1.,x,omega,ds,0.,NK);         // x = x + alpha*p + omega*s;
	field_cp(dr,1.,ds,-omega,das,0.,NK);       // r = s - omega*as;
	
	d0 = scalpr(dr,dr0,NK);
	beta = d0/d1*alpha/omega;
	d1 = d0;
	
	field_cp(dp,1.,dp,-omega,dap,0.,NK);
	field_cp(dp,1.,dr,beta,dp,0.,NK);          // p = r + beta*(p - omega*ap);
	
	res = sqrt(scalpr(dr,dr,NK));
	
	if(MSG>2)
	  printf("%d-te Iteration mit Residuum %.2e\n",k,res);
	if(res<akrit*Dmax){
	  if(MSG>1){
	    printf("NumIt = %d \n",k);// res = %6.2e, |D| = %6.2e ",k,res,Dmax);
	  }
	  break;
	}
      }
    }
  else
    {
      Dmax=1;
    }
  
  free( C   );
  free( ax  );
  free( r   );
  free( dr  );
  free( dr0 );
  free( dp  );
  free( as  );
  free( das );
  free( dap );
  free( ap  );
  free( ds  );
  
  return res/Dmax;
}



