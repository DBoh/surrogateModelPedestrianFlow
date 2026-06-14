/*!
  \file pFlowSIM.c

  \brief Pedestrian Flow Simulation
*/
#include <ped.h>
#include <externparam.h>
#include <string.h>


#if WEB==1
# include <emscripten.h>

void check_config();

typedef struct pFlowSIM{
  int l, LOOP, lc;
  REAL res_he, res_ke;
  REAL *rho, *rhovel, *f, *rhs;
  REAL *phi, *vel[2], *v, *bvel[2], *unu;
  REAL ped_time, omega;
} pFlowSIM;

pFlowSIM pFlow;

EMSCRIPTEN_KEEPALIVE
void setConfig(char *filename)
{

  FILE* file  = fopen("00Input/eFlow.dat", "w");
  fprintf(file, "00Input/%s", filename);
  fclose(file);

  sprintf(filename, "\0");

  // check_config();

  printf("In set config");
  standardvalues();
  ReadConfigFile();
}

EMSCRIPTEN_KEEPALIVE
int fileExists(char *filename){
  char path[] = "00Input/";
  strcat(path, filename);

  FILE *file;
  if ((file = fopen(path, "r")))
  {
      fclose(file);
      return 1;
  }
  return 0;
}

EMSCRIPTEN_KEEPALIVE
void check_config(){
   // Open file
  FILE *fptr;
  char c;

  fptr = fopen("00Input/eFlow.dat", "r");

  if (fptr == NULL)
  {
      printf("Cannot open file \n");
      exit(0);
  }

  // Read contents from file
  c = fgetc(fptr);
  while (c != EOF)
  {
      printf ("%c", c);
      c = fgetc(fptr);
  }
  printf("end\n");

  fclose(fptr);
}

EMSCRIPTEN_KEEPALIVE
void init()
{
  //-------------------------------------------------------------
  //Initialisierung: Data
  int i;
  time_t ttt;
  srand((unsigned) time(&ttt));

  //-------------------------------------------------------------
  //Initialisierung: Grid
  assign();
  AssignGrid(PED_bound);

  //-------------------------------------------------------------
  //Initialisierung: LGS

  pFlow.v        = (REAL *)calloc( NK , sizeof(REAL ));
  pFlow.rho      = (REAL *)calloc( NK , sizeof(REAL ));
  pFlow.unu      = (REAL *)calloc( NK , sizeof(REAL ));
  pFlow.ped_time = 0;

  for(i=0 ; i<2 ; i++)
    pFlow.vel[i]   = (REAL *)calloc( NK , sizeof(REAL ));

  MAT.Mass  = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  MAT.Stiff = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  MAT.MassExit = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  MAT.MassEntree = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  MAT.KonvExit = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  MAT.KonvEntree = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));

  //-------------------------------------------------------------
  //Initialisierung: Model

  //  PED_bound();
  //RA weg  ParamInit(pFlow.rho);
  SysMatrix(MAT.Mass,mass_elem);
  SysMatrix(MAT.Stiff,stiff_elem);
  BoundIntegral(MAT.MassExit,ZO.NKones,PED.numexit,PED.exit);
  BoundGradNuIntegral(MAT.KonvExit,ZO.NKones,PED.numexit,PED.exit);
  if(PED.numentree){
    BoundIntegral(MAT.MassEntree,ZO.NKones,PED.numentree,PED.entree);
    BoundGradNuIntegral(MAT.KonvEntree,ZO.NKones,PED.numentree,PED.entree);
  }
  ParamInit(pFlow.rho); // RA neu
  FuncInit(pFlow.v,pFlow.vel,pFlow.rho,pFlow.unu);

  ped_monitoring(pFlow.rho,pFlow.vel,pFlow.unu,0);

  MaxPhi(); // RA neu

  pFlow.LOOP = 0;
  pFlow.lc = 1;
}

EMSCRIPTEN_KEEPALIVE
void destruct()
{
  int i;
  FreeParam();
  free(pFlow.v);
  free(pFlow.phi);
  free(pFlow.rhs);
  free(pFlow.f);
  free(pFlow.unu);

  free(pFlow.rhovel);

  for(i=0 ; i<2 ; i++){
    free(pFlow.vel[i]);
    free(pFlow.bvel[i]);
  }
}

//***********************************//
// Getter and  Setter
//***********************************//

// ==== Scenario Parameter ==== //

EMSCRIPTEN_KEEPALIVE
float getNumPers()
{
  return video.TrackPerson;
}

EMSCRIPTEN_KEEPALIVE
REAL getAllPersons()
{
  return MONI.Mass;
}

EMSCRIPTEN_KEEPALIVE
REAL getEntreePersons()
{
  return MONI.Entree;
}

EMSCRIPTEN_KEEPALIVE
REAL getExitPersons()
{
  return MONI.Exit;
}

EMSCRIPTEN_KEEPALIVE
REAL getRhoMin()
{
  return MONI.Rhomin;
}

EMSCRIPTEN_KEEPALIVE
REAL getRhoMax()
{
  return MONI.Rhomax;
}

EMSCRIPTEN_KEEPALIVE
REAL getInfectedPersons()
{
  return MONI.SE_I_R;
}

EMSCRIPTEN_KEEPALIVE
REAL getSingleExitPersons(int index)
{
  if (index > PED.numexit) return 0;
  return MONI.Exits[index];
}

EMSCRIPTEN_KEEPALIVE
float getMaxTime()
{
  return PED.MaxTime;
}

EMSCRIPTEN_KEEPALIVE
float getCharLength()
{
  return PED.CL;
}

EMSCRIPTEN_KEEPALIVE
float getCharVelocity()
{
  return PED.CV;
}

EMSCRIPTEN_KEEPALIVE
float getCharDensity()
{
  return PED.CP;
}

EMSCRIPTEN_KEEPALIVE
int getNumSubdomains()
{
  return SDCV.NumSubdom;
}

EMSCRIPTEN_KEEPALIVE
float getSubdomainCharVelocity(int index)
{
  return PED.CVsd[index];
}

// Setter

EMSCRIPTEN_KEEPALIVE
void setNumPers(float numPers)
{
  if (numPers > 6000)
    numPers = 6000;
  else if (numPers < 2)
    numPers = 2;

  PED.Pinit = numPers;
}

EMSCRIPTEN_KEEPALIVE
void setMaxTime(float maxSimuTime)
{
  PED.MaxTime = maxSimuTime;
}

EMSCRIPTEN_KEEPALIVE
void setCharLength(float charLength)
{
  PED.CL = charLength;
}

EMSCRIPTEN_KEEPALIVE
void setCharVelocity(float charVelocity)
{
  PED.CV = charVelocity;
}

EMSCRIPTEN_KEEPALIVE
void setCharDensity(float charDensity)
{
  PED.CP = charDensity;
}

EMSCRIPTEN_KEEPALIVE
void setSubCharVelocity(int index, float charVelocity)
{
  PED.CVsd[index] = charVelocity;
}

// Setter

// ==== Infection Parameter ==== //
// Getter

EMSCRIPTEN_KEEPALIVE
int getSIR(int index)
{
  if (video.TrackInside[index])
    return video.SIRmode[index];
  else
    return -1;
}

EMSCRIPTEN_KEEPALIVE
float getPercentInfected(){
  return siragent.PI;
}

EMSCRIPTEN_KEEPALIVE
float getCriticalDistance(){
  return siragent.CD;
}

EMSCRIPTEN_KEEPALIVE
float getInfectionRate(){
  return siragent.IR;
}

EMSCRIPTEN_KEEPALIVE
float getResistanceTime(){
  return siragent.RT;
}

EMSCRIPTEN_KEEPALIVE
float getPercentRemoved(){
  return siragent.PR;
}
// Setter

EMSCRIPTEN_KEEPALIVE
void setPercentInfected(float percentInfected){
  siragent.PI = percentInfected;
}

EMSCRIPTEN_KEEPALIVE
void setCriticalDistance(float criticalDistance){
  siragent.CD = criticalDistance;
}

EMSCRIPTEN_KEEPALIVE
void setInfectionRate(float infectionRate){
  siragent.IR = infectionRate;
}

EMSCRIPTEN_KEEPALIVE
void setResistanceTime(float resistanceTime){
  siragent.RT = resistanceTime;
}

EMSCRIPTEN_KEEPALIVE
void setPercentRemoved(float percentRemoved){
  siragent.PR = percentRemoved;
}

// ==== Attractors ==== //
// Getter

EMSCRIPTEN_KEEPALIVE
int getNumAS()
{
  return PED.AS;
}

EMSCRIPTEN_KEEPALIVE
void getAttracs(int index, float* attracts)
{
  if (index > PED.AS) return;
  //x1,y1,x2,y2,attraction,distance of attr.
  attracts[0] = PED.ASx[0][index];
  attracts[1] = PED.ASx[1][index];
  attracts[2] = PED.ASx[2][index];
  attracts[3] = PED.ASx[3][index];
  attracts[4] = PED.ASx[4][index];
  attracts[5] = PED.ASx[5][index] * PED.CL;  // RA ???
  attracts[6] = PED.ASx[6][index];
}

// Setter

EMSCRIPTEN_KEEPALIVE
void setAttracAttrac(int index, float attrac)
{
  PED.ASx[4][index] = min_REAL(1,max_REAL(-1,attrac));
}

EMSCRIPTEN_KEEPALIVE
void setAttracRange(int index, float range)
{
  PED.ASx[5][index] = range / PED.CL;
}

EMSCRIPTEN_KEEPALIVE
void setAttracFreq(int index, float freq)
{
  PED.ASx[6][index] = freq;
}


// Agents

EMSCRIPTEN_KEEPALIVE
float getX(int index)
{
  if (index > video.TrackPersonMax || index < 0)
  {
    return -1;
  }
  return video.TrackXYPosition[0][index];
}

EMSCRIPTEN_KEEPALIVE
float getY(int index)
{
  if (index > video.TrackPersonMax || index < 0)
  {
    return -1;
  }
  return video.TrackXYPosition[1][index];
}

EMSCRIPTEN_KEEPALIVE
int getTPFlag(int index)
{
  if (index > video.TrackPerson || index < 0) return -1;
  return video.TrackInside[index];
}

EMSCRIPTEN_KEEPALIVE
float getGridX(int index)
{
  if (index < 0 || index >= NK)
  {
    return -1;
  }
  return XY[2 * index];
}

EMSCRIPTEN_KEEPALIVE
float getGridY(int index)
{
  if (index < 0 || index >= NK)
  {
    return -1;
  }
  return XY[2 * index + 1];
}

EMSCRIPTEN_KEEPALIVE
int getVertex0(int index)
{
  if (index > NT || index < 0)
    return -1;
  return tri[index].VNR[0];
}

EMSCRIPTEN_KEEPALIVE
int getVertex1(int index)
{
  if (index > NT || index < 0)
    return -1;
  return tri[index].VNR[1];
}

EMSCRIPTEN_KEEPALIVE
int getVertex2(int index)
{
  if (index > NT || index < 0)
    return -1;
  return tri[index].VNR[2];
}

EMSCRIPTEN_KEEPALIVE
float getRho(int index)
{
  if (index > NK || index < 0)
    return -1;
  return pFlow.rho[index];
}

EMSCRIPTEN_KEEPALIVE
int getNT() {
  return NT;
}

EMSCRIPTEN_KEEPALIVE
int getNK()
{
  return NK;
}

EMSCRIPTEN_KEEPALIVE
float getShoulder()
{
  return PED.Shoulder;
}

EMSCRIPTEN_KEEPALIVE
float getVelX(int index)
{
  if (index > NK || index < 0)
    return -1;
  return pFlow.vel[0][index];
}

EMSCRIPTEN_KEEPALIVE
float getVelY(int index)
{
  if (index > NK || index < 0)
    return -1;
  return pFlow.vel[1][index];
}

EMSCRIPTEN_KEEPALIVE
int getNumExit()
{
  return PED.numexit;
}

EMSCRIPTEN_KEEPALIVE
int getNumEntree()
{
  return PED.numentree;
}

EMSCRIPTEN_KEEPALIVE
void getExit(int index, float* exit)
{
  if (index > PED.numexit) return;
  exit[0] = PED.exit[0][index];
  exit[1] = PED.exit[1][index];
  exit[2] = PED.exit[2][index];
  exit[3] = PED.exit[3][index];
}

EMSCRIPTEN_KEEPALIVE
float getPedTime()
{
  return (float)pFlow.ped_time;
}

EMSCRIPTEN_KEEPALIVE
void getEntree(int index, float* entree)
{
  if (index > PED.numexit) return;
  entree[0] = PED.entree[0][index];
  entree[1] = PED.entree[1][index];
  entree[2] = PED.entree[2][index];
  entree[3] = PED.entree[3][index];
}

EMSCRIPTEN_KEEPALIVE
void setPersonEntreeFlow(float pef, float mpe)
{
  int l;
  for(l=0 ; l<PED.numentree ; l++){
    PED.EntreePersons[l] = pef;
    PED.MaxEntreePersons[l]= mpe;
  }
}

EMSCRIPTEN_KEEPALIVE
void setExitAttrac(int index, float attrac)
{
  PED.ExitAttraction[index] = min_REAL(1,max_REAL(0,attrac));
}

EMSCRIPTEN_KEEPALIVE
int peform_simu_step()
{
  pFlow.ped_time = pFlow.LOOP*PED.delta;
  REAL PeopleInside = MONI.Mass/video.TrackPersonMax;

  if (pFlow.ped_time<PED.MaxTime && ( PeopleInside>PED.empty*1.0e-02 || pFlow.ped_time<PED.MaxTime))
    {

    /* Helmholtz-Equation */
    pFlow.res_he = HelmholtzEquation(pFlow.LOOP,pFlow.rho,pFlow.v,PED_Helm_bound2vec,amul_bd_exit);

    /* update velocity */

    UpdateVelocity(pFlow.LOOP,pFlow.vel,pFlow.rho,pFlow.v,pFlow.unu);

    /* Kontinuity-Equation */

    pFlow.res_ke = ContinuityEquation(pFlow.LOOP,pFlow.rho,pFlow.vel,pFlow.unu,pFlow.v,PED_Kont_bound2vec,amul);

    pFlow.LOOP++;
    pFlow.ped_time = pFlow.LOOP * PED.delta;
    //    Persons  = TotalMass(MAT.Mass,pFlow.rho);

    ped_monitoring(pFlow.rho,pFlow.vel,pFlow.unu,pFlow.LOOP);

    TrackPoint(pFlow.rho,pFlow.vel,pFlow.LOOP);

    return 1;
  }
  else
  {
    return 0;
  }
}

EMSCRIPTEN_KEEPALIVE
void new_sim()
{
  destruct();
  init();
}

EMSCRIPTEN_KEEPALIVE
int intToAscii(int number) {
   return '0' + number;
}

EMSCRIPTEN_KEEPALIVE
int set_new_config(char *buf)
{
  int i;

  for (i = 0; i < 10; i++) {
    printf("%c ", intToAscii(buf[i]));
  }
  return 0;
}

EMSCRIPTEN_KEEPALIVE
int main(int argc, char* argv[])
{
  setConfig("ClassRoom.eflow");
  init();
  return 0;
}

#else

// ---------- startmain -------------------

void RhoTest(REAL *);
void TriangCheck();

int main(int argc, char *argv[])
{
  int i, LOOP=0, lc;
  REAL res_he, res_ke, Persons, PeopleInside;
  REAL *rho, *vel[2], *v, *unu;
  REAL ped_time;
  char msg1[100], msg2[100];
  time_t ttt;

  srand((unsigned) time(&ttt));
#if MSG
  sprintf(femlog,"%s.log",argv[0]);
  char msg[100];
  ttt = time(0);
  sprintf(msg,"%s gestartet: %s",argv[0],ctime(&ttt));
  WriteLog(msg,"w");
#endif

  //-------------------------------------------------------------
  //Initialisierung: Data

  standardvalues();
  ReadConfigFile();

  //-------------------------------------------------------------
  //Initialisierung: Grid

  assign();
  AssignGrid(PED_bound);

  // ***********************************************************

#if MSG==2
  TriangCheck();
#endif

  //-------------------------------------------------------------
  //Initialisierung: LGS

  v      = (REAL *)calloc( NK , sizeof(REAL ));
  rho    = (REAL *)calloc( NK , sizeof(REAL ));
  unu    = (REAL *)calloc( NK , sizeof(REAL ));

  for(i=0 ; i<2 ; i++)
    vel[i]   = (REAL *)calloc( NK , sizeof(REAL ));


  MAT.Mass  = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  MAT.Stiff = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  MAT.MassExit = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  MAT.KonvExit = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  if(PED.numentree){
    MAT.MassEntree = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
    MAT.KonvEntree = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  }

  //-------------------------------------------------------------
  //Initialisierung: Model

  //  PED_bound();
  SysMatrix(MAT.Mass,mass_elem);
  SysMatrix(MAT.Stiff,stiff_elem);
  BoundIntegral(MAT.MassExit,ZO.NKones,PED.numexit,PED.exit);
  BoundGradNuIntegral(MAT.KonvExit,ZO.NKones,PED.numexit,PED.exit);
  if(PED.numentree){
    BoundIntegral(MAT.MassEntree,ZO.NKones,PED.numentree,PED.entree);
    BoundGradNuIntegral(MAT.KonvEntree,ZO.NKones,PED.numentree,PED.entree);
  }
  ParamInit(rho);
  FuncInit(v,vel,rho,unu);

  ped_monitoring(rho,vel,unu,0);

#if EOC
  FieldCp(rho,0.,rho,0.,NK);
#endif

#if PLOT
  lc=OctavePlotAll(v,vel,rho,0,0);
#endif

  MaxPhi(); // das in ein init-file schieben

  /*****************************************************************/
  /*****************************************************************/
  LOOP=0; ped_time=0;
  Persons  = TotalMass(MAT.Mass,rho);
  PeopleInside = Persons/video.TrackPersonMax;
  while(ped_time<PED.MaxTime && PeopleInside>PED.empty*1.0e-02 ){
    /*****************************************************************/
    /* Helmholtz-Equation */

    /*
    EikonalMode=1;
    res_he=0;
    switch (EikonalMode)
      {
      case 1:
	FastSweeping(LOOP,rho,v,PED_Helm_bound2vec,amul_bd_exit); // v=phi hier
	UpdateVelocityPhi(LOOP,vel,rho,v,unu);
	break;
      case 2:
	if(LOOP==0)
	  QuasiEikonalGeom();
	QuasiEikonalSP(LOOP,rho,v);  // v = phi
	UpdateVelocityPhi(LOOP,vel,rho,v,unu);
	break;
      default:
	break;
      }
    */

    res_he = HelmholtzEquation(LOOP,rho,v,PED_Helm_bound2vec,amul_bd_exit);
    UpdateVelocity(LOOP,vel,rho,v,unu);

    /*
      break;
      }
    */

    /* Kontinuity-Equation */
    res_ke = ContinuityEquation(LOOP,rho,vel,unu,v,PED_Kont_bound2vec,amul_bd_entree);

    /****************************************************************/
    /****************************************************************/
    /* output */
    LOOP++;
    ped_time = LOOP*PED.delta;
    Persons  = TotalMass(MAT.Mass,rho);
    PeopleInside = Persons/video.TrackPersonMax;

    if(FieldMax(rho,NK)>=1)
      ErrorMessage("Density > 1","pFlowSIM");

#if MSG
    if(fps_check(LOOP,PED.delta,PED.plotfreq)){
      sprintf(msg1,"LOOP=%7d time=%.2e",LOOP,ped_time);
      sprintf(msg2,"Res(HE)=%.2e Res(KE)=%.2e",res_he,res_ke);
      Message(msg1,'-',msg2);
    }
#endif

#if PLOT
    lc=OctavePlotAll(v,vel,rho,LOOP,lc);
#endif

    ped_monitoring(rho,vel,unu,LOOP);

#if TP
    TrackPoint(rho,vel,LOOP);
#endif

  }

  /* cleaning up */
  FreeParam();

  free(v);
  free(rho);
  free(unu);
  for(i=0 ; i<2 ; i++)
    free(vel[i]);


  return 0;
}

void RhoTest(REAL *rho)
{
  REAL *MG, Door[4], V1, V2, V3, V4;
  MG = (REAL *)calloc(NK*bandwidth , sizeof(REAL ));

  // unten
  Door[0] = 0.; Door[1] = 0.;
  Door[2] = 3.5433; Door[3] = 0.;
  BoundSegIntegral(MG,ZO.NKones,Door);
  V1 = VecVecIntegral(MG,rho,ZO.NKones);
  printf("unten: grad rho*nu = %.2e\n",V1);
  // oben
  Door[0] = 3.5433; Door[1] = 18.0709;
  Door[2] = 0.; Door[3] = 18.0709;
  BoundSegIntegral(MG,ZO.NKones,Door);
  V2 = VecVecIntegral(MG,rho,ZO.NKones);
  printf("oben: grad rho*nu = %.2e\n",V2);
  // links
  Door[0] = 0.; Door[1] = 18.0709;
  Door[2] = 0.; Door[3] = 0.;
  BoundSegIntegral(MG,ZO.NKones,Door);
  V3 = VecVecIntegral(MG,rho,ZO.NKones);
  printf("links: grad rho*nu = %.2e\n",V3);
  // rechts
  Door[0] = 3.5433; Door[1] = 0.;
  Door[2] = 3.5433; Door[3] = 18.0709;
  BoundSegIntegral(MG,ZO.NKones,Door);
  V4 = VecVecIntegral(MG,rho,ZO.NKones);
  printf("rechts: grad rho*nu = %.2e\n",V4);
  printf("Summe:  grad rho*nu = %.2e\n",V1+V2+V3+V4);

  free(MG);
}


void TriangCheck()
{
  extern Basis phi;
  extern GE *tri;
  extern GN *node;
  extern int NT, NK, *nach;

  int k, i, j, kj, ki, treffer, ii;
  int j1, j2, j3, i1, i2, i3, Err=0, kn, l, lg, found, km;
  char msg1[100];

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
	  //	  WriteLog("E: Korrupte Nachbarschaftsbeziehungen!\n",1);
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
	      //	      WriteLog("E: Korrupte Knotennumerierung! (1)\n",1);
	      printf("Error! Read the log-file!\n");
	      Err++;
	    }
	  } else if(phi.DEG==2){
	    j2 = tri[k].VNR[(j+1)%3+3];
	    i2 = tri[kj].VNR[(ii+1)%3+3];
	    if(i1!=j1 || i2!=j2 || i3!=j3){
	      //	      WriteLog("E: Korrupte Knotennumerierung! (2)\n",1);
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
	//WriteLog("E: Korrupte Nachbardreiecke an Knoten! \n",1);
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
	//WriteLog("E: Korrupte Nachbarknoten an Knoten! \n",1);
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
      //WriteLog("E: Korrupte Zuweisung in der Matrixposition! (1)\n",1);
      printf("Error! Read the log-file!\n");
      Err++;
    }
    for(l=node[i].pointmat ; l<node[i+1].pointmat ; l++){
      kj = nach[l];
      if(kj<0 || kj>=NK){
	//WriteLog("E: Korrupte Zuweisung in der Matrixposition! (2)\n",1);
	printf("Error! Read the log-file!\n");
	Err++;
      }
    }
  }

  sprintf(msg1,"%d",Err);
  Message("Errors in TriangCheck",'.',msg1);

}



#endif


