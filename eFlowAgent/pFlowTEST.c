/*!
  \file pFlow2d.c

  \brief Pedestrian Flow Simulation
*/
#include <ped.h>
#include <externparam.h>
#include <time.h>

DAT PED;
char femlog[200], msg1[100], msg2[100];
VIDEO video;

void pFlowTESThelp(char *);
int DoorCheck(int );
void TriangCheck();
void TransformCheck();
void MassCheck(char *, int , int );
void SearchTriTest();
void Quadrangulationplot();

// v = x^3-x^2+x
REAL dW_Exact(REAL t, REAL x, REAL y)
{
  REAL dv, bdv;
  dv = 3*x*x-2*x+1;
  bdv = sqrt(dv*dv+EpsReg*EpsReg);
  
  return dv/bdv;
}
REAL d2W_Exact(REAL t, REAL x, REAL y, int flag)
{
  REAL dv, bdv;
  dv = 2*EpsReg*EpsReg*(3*x - 1);
  bdv = EpsReg*EpsReg + (3*x*x - 2*x + 1)*(3*x*x - 2*x + 1);
  bdv = pow(bdv,1.5);
  
  if(flag) return dv/bdv;
  else return 0;
}

REAL clement(REAL t, REAL x, REAL y){return x*y;}
REAL dclement(REAL t, REAL x, REAL y, int flag){
  if(flag==0) return y;
  else return x;}


int main(int argc, char *argv[])
{
  extern DAT PED; 
  extern int numdOmega, *dOmega[4], QuadN, QuadM;
  extern TRI refine;
  extern GN *node;
  extern GE *tri;
  extern int NK, NT, bandwidth;
  extern REAL *XY;
  extern MATRIX MAT;
  extern QUADRA *quad;
  
  int i, l, debug;
  REAL *zerovec[2], *onevec[2], *zero, *one, *rho, *rhovel, *f, *rhs;
  REAL *phi, *vel[2], *v, *bvel[2], *unu, *dve[2], *dvh[2];
  time_t ttt;
  char KOM[100], KOT[100];
  char NOM[100], MODE[100];
  int FUNK[3];

  sprintf(NOM,"%s",argv[0]);
  debug=1;
  /* no args for debug mode */
  if(debug){
    sprintf(KOM,"M");
    sprintf(KOT,"I");
    sprintf(MODE,"-0");

    FUNK[0]=1; FUNK[1]=0; FUNK[2]=0; 
  } else {
  
    if(argc==1 ||
       !strcmp(argv[1],"-h") ||
       !strcmp(argv[1],"-help") ||
       !strcmp(argv[1],"--help")
       ) pFlowTESThelp(NOM);

    sprintf(MODE,"%s",argv[1]);
  }
  
  srand((unsigned) time(&ttt));
  sprintf(femlog,"%s.log",NOM);

  #if MSG==1
  Message("\n",'*'," Program started");
  Message("Logfile: ",'.',femlog);
  #endif

  //-------------------------------------------------------------
  //Initialisierung: Data
  standardvalues();
  ReadFEMData();
  ReadPEDParam();

  //-------------------------------------------------------------
  //Initialisierung: Grid
  assign();
  AssignGrid(PED_bound,&numdOmega);

  //-------------------------------------------------------------
  //Initialisierung: LGS

  zero   = (REAL *)calloc( NK , sizeof(REAL ));
  one    = (REAL *)calloc( NK , sizeof(REAL ));
  rho    = (REAL *)calloc( NK , sizeof(REAL ));
  unu    = (REAL *)calloc( NK , sizeof(REAL ));
  v      = (REAL *)calloc( NK , sizeof(REAL ));

  for(i=0 ; i<2 ; i++){
    zerovec[i]  = (REAL *)calloc( NK , sizeof(REAL ));
    onevec[i]   = (REAL *)calloc( NK , sizeof(REAL ));
    FieldCp(onevec[i],0.,onevec[i],1.,NK);
    vel[i]      = (REAL *)calloc( NK , sizeof(REAL ));
    dve[i]      = (REAL *)calloc( NK , sizeof(REAL ));
    dvh[i]      = (REAL *)calloc( NK , sizeof(REAL ));
  }
  FieldCp(one,0.,one,1.,NK);

  MAT.Mass  = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  MAT.Stiff = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  
  PED_bound();
  SysMatrix(MAT.Mass,mass_elem);
  SysMatrix(MAT.Stiff,stiff_elem);
  ParamInit(rho);
  FuncInit(v,vel,rho,unu);

  ped_monitoring(rho,unu,0);
 
  slip_boundary(vel);
  u_nu(unu,vel);


  if(debug){
    octaveplot(v,"W",0.,-1.,0);
    
    DuhClementVec(dvh,v);
    ErrorFunc(0.,dvh[0],dW_Exact,d2W_Exact,"CW");
    
    octaveplot(dvh[0],"dW",0.,-1.,0);
    exit(0);
  }

  //-------------------------------------------------------------
  if(!strcmp("-0",MODE)){
    Message("Teste Gitterbeziehungen",' ',"");
    TriangCheck();
  } else if(!strcmp("-1",MODE)){
    Message("Teste BoundFlowIntegral - Ausgaenge",' ',"");
    DoorCheck(1);
  } else if(!strcmp("-2",MODE)){
    Message("Teste BoundFlowIntegral - Eingaenge",' ',"");
    DoorCheck(0);
  } else if(!strcmp("-3",MODE)){
    // NOM MODE KOM KOT

    if(argc>=3) sprintf(KOM,"%s",argv[2]);
    if(argc>=4) sprintf(KOT,"%s",argv[3]);
    if(!strcmp("O",KOT) && argc!=4) {
      //HelpMatrixTest(NOM,MODE);
    } else if(!strcmp("I",KOT) && argc==5) {
      FUNK[0] = atoi(argv[4]);
      FUNK[1] = FUNK[0];
      FUNK[2] = FUNK[0];
    } else if(!strcmp("I",KOT) && argc==7) {
      FUNK[0] = atoi(argv[4]);
      FUNK[1] = atoi(argv[5]);
      FUNK[2] = atoi(argv[6]);
    } else if(!(!strcmp("O",KOT) && argc==4)){
      //HelpMatrixTest(NOM,MODE);
    }
    //    MatrixIntTest(KOM,KOT,FUNK);

  } else if(!strcmp("-5",MODE)){
    Message("Teste Transformation auf Referenzdreieck",' ',"");
    TransformCheck();
  } else if(!strcmp("-6",MODE)){
    if(argc!=5){
      printf("usage: %s %s Monitoring-file startline endline\n",NOM,MODE);
      exit(0);
    }
    Message("Teste Monitoringdata (Masseerhaltung)",' ',"");
    MassCheck(argv[2],atoi(argv[3]),atoi(argv[4]));
  } else if(!strcmp("-7",MODE)){
    Message("Teste SearchTri",' ',"");
    SearchTriTest();
    Quadrangulationplot();
  } else {
    ErrorMessage("unknown test","pFlowTEST");
  }
  
  //-------------------------------------------------------------
  

  return 0;
}


void Quadrangulationplot()
{
  int i, j, ij;
  REAL hN, hM;
  FILE *fp;

  fp = fopen("GnuPlot/Quadrangulation.dat","w");
  if(!fp) ErrorMessage("Can't open file","Quadrangulationplot.dat");
  hM = refine.boundy_ur/QuadM;
  hN = 1./QuadN;
  
  for(i=0 ; i<QuadM+1 ; i++)
    fprintf(fp,"set arrow from %f,%f to %f,%f nohead\n",0.,i*hM,1.,i*hM);
  for(j=0 ; j<QuadN+1 ; j++)
    fprintf(fp,"set arrow from %f,%f to %f,%f nohead\n",j*hN,0.,j*hN,refine.boundy_ur);
  
  fclose(fp);

  fp = fopen("GnuPlot/QuadHist.dat","w");
  if(!fp) ErrorMessage("Can't open file","QuadHist.dat");

  for(ij=0 ; ij<QuadN*QuadM ; ij++){

    fprintf(fp,"%d %d\n",ij,quad[ij].Qnum);

  }

  fclose(fp);

}



void SearchTriTest()
{
  int k, kk, count=0, l;
  REAL xx, yy;
  char msg[100];
  
  for(k=0 ; k<NT ; k++){
    // Mittlepunkt
    xx = (XY[2*tri[k].VNR[0]]+XY[2*tri[k].VNR[1]]+XY[2*tri[k].VNR[2]])/3;
    yy = (XY[2*tri[k].VNR[0]+1]+XY[2*tri[k].VNR[1]+1]+XY[2*tri[k].VNR[2]+1])/3;

    kk = search_tri(0,xx,yy,0);

    if(k!=kk){
      count++;
      //      Message("found triangle doesn't match",'.',"");
    }
    
    // Kante
    for(l=0 ; l<3 ; l++){
      xx = (XY[2*tri[k].VNR[l]]+XY[2*tri[k].VNR[(l+1)%3]])/2;
      yy = (XY[2*tri[k].VNR[l]+1]+XY[2*tri[k].VNR[(l+1)%3]+1])/2;

      kk = search_tri(0,xx,yy,0);

      if(k!=kk && tri[k].NEIGH[(l+2)%3]!=kk){
	count++;
	//      Message("found triangle doesn't match",'.',"");
      }
    }

  }

  sprintf(msg,"%d",count);
  Message("wrong computations",'.',msg);

}

void pFlowTESThelp(char *pname)
{
  Message("\nusage",' ',"\n");

  sprintf(msg1,"%s -0:",pname);
  sprintf(msg2,"Grid check");
  Message(msg1,' ',msg2);

  sprintf(msg1,"%s -1:",pname);
  sprintf(msg2,"Exit check");
  Message(msg1,' ',msg2);

  sprintf(msg1,"%s -2:",pname);
  sprintf(msg2,"Entree check");
  Message(msg1,' ',msg2);

  sprintf(msg1,"%s -3:",pname);
  sprintf(msg2,"Matrix check");
  Message(msg1,' ',msg2);

  sprintf(msg1,"%s -4:",pname);
  sprintf(msg2,"LGS check");
  Message(msg1,' ',msg2);

  sprintf(msg1,"%s -5:",pname);
  sprintf(msg2,"Transform check");
  Message(msg1,' ',msg2);

  sprintf(msg1,"%s -6:",pname);
  sprintf(msg2,"Monitoring/conservation of mass check");
  Message(msg1,' ',msg2);

  sprintf(msg1,"%s -7:",pname);
  sprintf(msg2,"search_tri - Test");
  Message(msg1,' ',msg2);


  printf("\n");
  exit(0);
  
}


int DoorCheck(int flag)
{
  int i;
  REAL *MA, *one, *ax, GammaLen;
  REAL gam=0.;
  
  MA   = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  one  = (REAL *)calloc( NK , sizeof(REAL ));
  ax   = (REAL *)calloc( NK , sizeof(REAL ));

  FieldCp(one,0.,one,1.,NK);
  switch(flag){
  case 0:
    BoundFlowIntegral(MA,one,PED.numentree,PED.entree);
    sprintf(msg1,"Entree length");
    for(i=0 ; i<PED.numentree ; i++)
      gam += sqrt(pow(PED.entree[0][i]-PED.entree[2][i],2.)
		  +pow(PED.entree[1][i]-PED.entree[3][i],2.));
    break;
  case 1:
    BoundFlowIntegral(MA,one,PED.numexit,PED.exit);
    sprintf(msg1,"Exit length");
    for(i=0 ; i<PED.numexit ; i++)
      gam += sqrt(pow(PED.exit[0][i]-PED.exit[2][i],2.)
		  +pow(PED.exit[1][i]-PED.exit[3][i],2.));
    break;
  default:
    ErrorMessage("unknown door-type","DoorCheck");
  }
  
  GammaLen = VecVecIntegral(MA,one,one); 

  sprintf(msg2,"%.2e",GammaLen);
  Message(msg1,'.',msg2);
  sprintf(msg2,"%.2e",gam);
  Message("Sollwert",'.',msg2);
  sprintf(msg2," %.2e %.2e",refine.gridwidth,fabs(gam-GammaLen));
  Message("Differenz",'.',msg2);
}



void TriangCheck()
{
  extern Basis phi;
  extern GE *tri;
  extern GN *node;
  extern int NT, NK, *nach;
  extern REAL *XY;

  int k, i, j, kj, ki, treffer, ii;
  int j1, j2, j3, i1, i2, i3, Err=0, kn, l, lg, found, km;
    
  for(k=0 ; k<NT ; k++){
    for(j=0 ; j<3 ; j++){
      kj = tri[k].NEIGH[j];

      if(kj>-1){
	// Nachbarn sehen sich gegenseitig
	treffer = 0;
	for(i=0 ; i<3 ; i++){
	  ki = tri[kj].NEIGH[i];
	  if(ki==k) {
	    treffer = 1;
	    ii = i;
	  }
	}
	if(treffer==0){
	  WriteLog("E: Korrupte Nachbarschaftsbeziehungen!\n",1);
	  printf("Error! Read the log-file!\n");
	  Err++;
	} else {

	  // Nachbarn haben gleiche Knoten gemeinsam
	  j1 = tri[k].VNR[(j+1)%3];
	  j3 = tri[k].VNR[(j+2)%3];

	  i1 = tri[kj].VNR[(ii+2)%3];
	  i3 = tri[kj].VNR[(ii+1)%3];

	  if(phi.DEG==1){
	    if(i1!=j1 || i3!=j3){
	      WriteLog("E: Korrupte Knotennumerierung! (1)\n",1);
	      printf("Error! Read the log-file!\n");
	      Err++;
	    }
	  } else if(phi.DEG==2){
	    j2 = tri[k].VNR[(j+1)%3+3];
	    i2 = tri[kj].VNR[(ii+1)%3+3];
	    if(i1!=j1 || i2!=j2 || i3!=j3){
	      WriteLog("E: Korrupte Knotennumerierung! (2)\n",1);
	      printf("Error! Read the log-file!\n");
	      Err++;
	    }
	  }
	}
      }
    }
  }


  for(i=0 ; i<NK ; i++){
    kj = node[i].NEIGH_TRI[0];
    for(k=1 ; k<= kj ; k++){
      kn = node[i].NEIGH_TRI[k];
      found=0;
      for(l=0 ; l<3 ; l++){
	lg = tri[kn].VNR[l];
	if(lg==i)
	  found=1;
	
      }
      if(found==0){
	WriteLog("E: Korrupte Nachbardreiecke an Knoten! \n",1);
	printf("Error! Read the log-file! \n");
	Err++;
      }
    }
  }


  for(i=0 ; i<NK ; i++){
    kj = node[i].NEIGH_NODE[0];
    for(k=1 ; k<= kj ; k++){
      kn = node[i].NEIGH_NODE[k];
      km = node[kn].NEIGH_NODE[0];
      found=0;
      for(l=1 ; l<=km ; l++){
	lg = node[kn].NEIGH_NODE[l];
	if(lg==i)
	  found=1;
      }
      if(found==0){
	WriteLog("E: Korrupte Nachbarknoten an Knoten! \n",1);
	printf("Error! Read the log-file! \n");
	Err++;
      }
    }
  }


  for(i=0 ; i<NK ; i++){
    kj = node[i].pointmat;
    kn = node[i].NEIGH_NODE[0];
    km = node[i+1].pointmat;
    if(km-kj!=kn+1){
      WriteLog("E: Korrupte Zuweisung in der Matrixposition! (1)\n",1);
      printf("Error! Read the log-file!\n");
      Err++;
    }
    for(l=node[i].pointmat ; l<node[i+1].pointmat ; l++){
      kj = nach[l];
      if(kj<0 || kj>=NK){
	WriteLog("E: Korrupte Zuweisung in der Matrixposition! (2)\n",1);
	printf("Error! Read the log-file!\n");
	Err++;
      }
    }
  }

  sprintf(msg1,"%d",Err);
  Message("Errors in TriangCheck",'.',msg1);
  
}


void TransformCheck()
{
  extern int NT;

  int k, m, mm, count=0, l;
  REAL st[3]={1./6.,2./3.,1./6.}, xd[2], x[2], xd2[2];
  REAL eps=1.E-04, error=0.;
  
  for(k=0 ; k<NT ; k++){
    for(m=0 ; m<3 ; m++){
      mm=2-m;
      xd[0] = st[mm%3];
      xd[1] = st[(mm+1)%3];
      transform2_point(xd,k,x);
      transform_point(x,k,xd2);
      error=0.;
      for(l=0 ; l<2 ; l++)
	error += pow((xd[l]-xd2[l]),2.);
      
      error = sqrt(error);
      if(error>eps){
	printf("Error in check_transform: %f\n",error);
	count ++;
      }
    }
  }

  sprintf(msg1,"%d",count);
  Message("Errors in TransformCheck",'.',msg1);

}


void MassCheck(char *monidata, int start, int end)
{
  int i, step;
  float time, mass, exit, entree, initmass, massold=0;
  char linebuf[100];
  FILE *fp;

  step = (int)((end-start)/10+1);
  
  
  printf("Zeit: Mass = Startmasse + Zulauf - Ablauf | h Error\n");
    
  fp = fopen(monidata,"r");
  if(!fp) ErrorMessage("Can't open file","Masscheck");
  for(i=0 ; i<end ; i++){
    fgets(linebuf,sizeof(linebuf),fp);
    massold=mass;
    sscanf(linebuf,"%f %f %f %f",&time,&mass,&exit,&entree);
    
    if(i==start) initmass = mass;

    if(i>start && i%step==0) {
      printf("%8.2f: %8.2f = %8.2f + %8.2f - %8.2f | %9.2e %9.2e \n",time,mass,initmass,entree,exit,refine.gridwidth,fabs(initmass+entree-exit-massold));
    }

  }
  fclose(fp);
    

}


