#include <ped.h>

extern int NT, NK;
extern GE *tri;
extern Basis phi;
extern REAL *XY;
extern GN *node;
extern QUADRATUR QUAD;
extern MATRIX MAT;
extern DAT PED;
extern NEUTRALS ZO;


void ContRHS(REAL time, REAL *rhs, REAL *rho)
{

  amul(MAT.Mass,rho,rhs);
  
#if EOC
  FieldCp(rhs,0.0,rhs,0.0,NK); // rho_t=0;
  REAL *rhs1, *ME, *gCR;
  ME   = (REAL *)calloc(NK*bandwidth , sizeof(REAL ));
  rhs1 = (REAL *)calloc(NK , sizeof(REAL ));
  gCR  = (REAL *)calloc(NK , sizeof(REAL ));
  
  G_Cont_Exact_Vec(time,gCR);
  if(PED.numentree)
    P_In_Exact_Vec(time,PIn);
  BoundIntegral(ME,ZO.NKones,PED.numentree,PED.entree);
		
  amul(MAT.Mass,gCR,rhs1);
  amul(ME,PIn,rhs2);

  field_cp(rhs,1.,rhs,PED.delta,rhs1,0.,NK);
  field_cp(rhs,1.,rhs,PED.delta,rhs2,0.,NK);

  free(gCR); free(rhs1); free(ME);
#endif


  if(0 && PED.numentree){
    
    int l, i, ei;
    REAL *rhs2, *PIn;

    PIn = (REAL *)calloc(NK , sizeof(REAL ));
    rhs2 = (REAL *)calloc(NK , sizeof(REAL ));
    
    for(l=0 ; l<PED.numentree ; l++){

      if(PED.MaxEntreePersons[l]>PED.EntreePersons[l]*time)
	{
      
	  for(i=0 ; i<NK ; i++){
	    ei = ChkEntree(i);
	    if(ei-1==l){
	      PIn[i] = PED.EntreePersons[ei-1]*PED.CT/PED.CP;
	    }
	  }
	}
    }
    
    amul(MAT.MassEntree,PIn,rhs2);

#if PLOT==2
    i = floor(time/PED.delta);
    octaveplot(PIn,"PIN",0.,-1.,i);
    octaveplot(rhs2,"RHS2",0.,-1.,i);
#endif
    
    field_cp(rhs,1.,rhs,PED.delta,rhs2,0.,NK);

    free(rhs2); 
    free(PIn);

  }
  
}




