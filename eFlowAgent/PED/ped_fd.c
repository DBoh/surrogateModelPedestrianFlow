/*!
  \file ped_fd.c
  \brief read fundamental diagram
*/
#include <ped.h>

extern DAT PED;
extern SUBDOM SDCV, Att;
extern REAL *XY;
extern int NT, NK;

int DfMin(REAL *Df, int N, int ia, int ib);



/*! \brief returns vector f(rho) depending on the normalised Fundamental diagram */
void FD_Vec(REAL *v, REAL *rho, int l)
{
  int i;

  for(i=0 ; i<NK ; i++)
    v[i] = FD_Value(rho[i],l,XY[2*i],XY[2*i+1],1)*SDCV.Kvec[i]*Att.Kvec[i];

}

/*! \brief returns value f(x) depending on the normalised Fundamental diagram */
REAL FD_Value(REAL rho, int l, REAL xx, REAL yy, int flag)
{
  int i;
  REAL cp=1., cv=1., gamma=3.21868e-01, goatin=2, rhoreg, rhoregsq;
  REAL eps=1.0e-08, dummy, afac=0.4, alpha, fd_out=0;

  //  if(fabs(rho)<=eps) return 1-l;
  PED.FD_SpecificPower=0.5;

#if EOC
  rhoreg = rho;
#else
  rhoregsq = pow(rho,2.)+pow(PED.EPS2,2.);
  rhoreg = sqrt(rhoregsq);
#endif

  switch (PED.FD){ // TODO enum
  case 0:
    if(l==0)
      fd_out = 1.-rhoreg;
    else
      fd_out = -1.;
    break;
    PED.FD_SpecificPower=0.25;
  case 1:
    alpha=log(1-afac)/gamma+1;
    dummy=exp(gamma*(alpha-1.0/rhoreg));
    if(l==0)
      fd_out = 1.-dummy;
    else
      fd_out = -dummy*gamma/rhoregsq;
    break;
    PED.FD_SpecificPower=0.18;
  case 2:
    if(l==0)
      fd_out = cv*(1.0-exp(-gamma*(1.0/rhoreg-1.0/cp)));
    else
      fd_out = -cv*exp(-gamma*(1.0/rhoreg-1.0/cp))*gamma*(1.0/rhoregsq);
    break;

  case 3:
    if(l==0)
      fd_out = cv*exp(-goatin*(rhoreg/cp));
    else
      fd_out = -cv*goatin/cp*exp(-goatin*(rhoreg/cp));
    break;

  case 4:
    if(l==0)
      fd_out = fd_weidmann_linear(PED.FundamentalDiagram,PED.FDnk,rho);
    else
      fd_out = fd_weidmann_linear(PED.dFundamentalDiagram,PED.FDnk,rho);
    break;

  default:  /* Validierung PeTrack 
https://www.researchgate.net/figure/Pedestrian-fundamental-diagram-where-a-shows-the-relationship-between-density-k-and_fig5_286513143, Gln (1)
*/
    if(l==0) {
      rhoreg = min_REAL(rhoreg,1.);
      fd_out = cv*(1-exp(-0.3*(1./rhoreg-1.)));
    } else
      fd_out = -cv*2.1*exp(0.3 - 2.1/cp/rhoreg)/cp/rhoregsq;
    break;

  }

  return max_REAL(0.,fd_out);
}




REAL fd_weidmann_linear(REAL *FD[2], int N, REAL rho)
{
  int i;
  REAL out=0., y2, y1, x2, x1;

  //  if(rho>=0.0 && rho<=FD[0][0]) return FD[1][0];
  if(rho<=0) return FD[1][0];
  if(rho>=1) return 0;

  for(i=0 ; i<N-1 ; i++)
    if(rho>=FD[0][i] && rho<=FD[0][i+1] ){
      x1 = FD[0][i];
      x2 = FD[0][i+1];
      y1 = FD[1][i];
      y2 = FD[1][i+1];
      out = (y2*(rho-x1)+y1*(x2-rho))/(x2-x1);
      return out;
    }

  return out;
}



/*! \brief same routine as ReadFundamentalDiagram in ../Fundametaldiagramm/C
 */
int ReadFDData(char *datname, REAL scal)
{
  int i, N, l, ll;
  FILE *fp;
  char msg[100], linebuf[100];
  REAL xmax=0, ymax=0;
  REAL d1, d2;
  REAL dummy1, dummy2;

  N=91;
  for(l=0 ; l<2 ; l++)
    PED.dFundamentalDiagram[l] = (REAL *)calloc(N , sizeof(REAL ));
  

  for(i=0 ; i<N; i++){
    if(PED.FundamentalDiagram[0][i]>xmax) xmax=PED.FundamentalDiagram[0][i];
    if(PED.FundamentalDiagram[1][i]>ymax) ymax=PED.FundamentalDiagram[1][i];
  }


  for(i=0 ; i<N; i++){
    PED.FundamentalDiagram[0][i] /= xmax;
    PED.FundamentalDiagram[1][i] /= ymax/scal;
  }

  FieldCp(PED.dFundamentalDiagram[0],1.,PED.FundamentalDiagram[0],0.,N);

  for(i=1 ; i<N-1; i++){
    dummy1  = PED.FundamentalDiagram[1][i+1]-PED.FundamentalDiagram[1][i];
    dummy1 /= PED.FundamentalDiagram[0][i+1]-PED.FundamentalDiagram[0][i];
    dummy2  = PED.FundamentalDiagram[1][i]-PED.FundamentalDiagram[1][i-1];
    dummy2 /= PED.FundamentalDiagram[0][i]-PED.FundamentalDiagram[0][i-1];

    PED.dFundamentalDiagram[1][i] = 0.5*(dummy1+dummy2);
  }

  dummy1  = PED.FundamentalDiagram[1][1]-PED.FundamentalDiagram[1][0];
  dummy1 /= PED.FundamentalDiagram[0][1]-PED.FundamentalDiagram[0][0];
  dummy2  = PED.FundamentalDiagram[1][N-1]-PED.FundamentalDiagram[1][N-2];
  dummy2 /= PED.FundamentalDiagram[0][N-1]-PED.FundamentalDiagram[0][N-2];

  PED.dFundamentalDiagram[1][0]   = dummy1;
  PED.dFundamentalDiagram[1][N-1] = dummy2;

  return N;
}


/*!
  \brief Stromdichte als Konstante auf einer Strecke

  \detail so dass eine gew\"unschte Durchflussrate auf der Strecke \f$\overline{ab}\f$ gegeben ist. Eingabe ist die Strecke (Eingang \f$\Gamma_E\f$)

  \f[\texttt{Segment[4]}=\{a_x,a_y,b_x,b_y\}\,,\f]

  die Anzahl \texttt{Personen} (dimensionslos) pro \f$T\f$ (sec) und die M\"oglichkeit zu w\"ahlen, ob man wenig Personen schnell durchschickt (\texttt{Fast=1}) oder viele Personen langsam kommen (\texttt{Fast=0}).
*/
//PED.FundamentalDiagram
REAL CurrentDensity(REAL Segment[4], REAL FlowRate, int Fast)
{
  REAL *SL, *Df, *FD[2];
  REAL SLmax=0, Pmax, GE, Sx, Sy;
  REAL Personen, eps=1.0e-09, rho, hfd=1.0/(PED.FDnk-1);
  int IntLoc[2][2], FDnk;
  int i, l, imax=0, id1, id2, dmin;
  char msg1[100];

  FDnk = PED.FDnk;

  SL = (REAL *)calloc(FDnk , sizeof(REAL ));
  Df = (REAL *)calloc(FDnk , sizeof(REAL ));

  GE   = sqrt(pow(Segment[0]-Segment[2],2.)+pow(Segment[1]-Segment[3],2.));

  for(l=0 ; l<2 ; l++){
    FD[l] = (REAL *)calloc(PED.FDnk , sizeof(REAL ));
    //    FieldCp(FD[l],1.,PED.FundamentalDiagram[l],0.,PED.FDnk);
  }

  for(i=0 ; i<PED.FDnk ; i++){
    FD[0][i] = hfd*i;
    FD[1][i] = FD_Value(hfd*i,0,0.,0.,0);
  }

  for(i=0 ; i<FDnk ; i++){
    SL[i] = FD[0][i]*FD[1][i];
    if(SL[i]>SLmax){SLmax = SL[i]; imax = i;}
  }

  IntLoc[1][0]=0;    IntLoc[1][1]=imax;
  IntLoc[0][0]=imax; IntLoc[0][1]=FDnk;

  Personen = FlowRate*PED.CP*PED.CT*PED.CV*PED.CL*GE;
  Pmax    = SLmax*PED.CL*PED.CP*PED.CT*PED.CV*GE;
#if WMSG
  if(Pmax<Personen){
    sprintf(msg1,"Too high flowrate, max possible %.2f",Pmax);
    ErrorMessage(msg1,"CurrentDensity");
  }
#endif
  FlowRate = max_REAL(0.,min_REAL(FlowRate,SLmax));

  for(i=0 ; i<FDnk ; i++)
    Df[i] = fabs(SL[i]-FlowRate);


  if(fabs(FlowRate-SLmax)<eps)
    rho   = FD[0][imax];
  else {

    dmin = DfMin(Df,FDnk,IntLoc[Fast][0],IntLoc[Fast][1]);

    Sx = FD[0][dmin];
    Sy = SL[dmin];

    if(fabs(Sy-FlowRate)<eps){
      rho = Sx;
    } else {
      if((Sy>FlowRate && Fast) || (Sy<FlowRate && !Fast)){
  id1 = dmin-1;
  id2 = dmin;
      } else if((Sy<FlowRate && Fast) || (Sy>FlowRate && !Fast)){
  id1 = dmin;
  id2 = dmin+1;
      }
      rho = FD[0][id1]+(FlowRate-SL[id1])/(SL[id2]-SL[id1])*(FD[0][id2]-FD[0][id1]);
    }

  }

  #if PLOT
  FILE *fp;
  fp = fopen("GnuPlot/weidmann.dat","w");
  for(i=0 ; i<FDnk ; i++)
    fprintf(fp,"%f %f\n",FD[0][i],SL[i]);
  fclose(fp);

  fp = fopen("GnuPlot/FD_Punkte.dat","w");
  fprintf(fp,"%f %f\n",rho,FlowRate);
  fclose(fp);

  fp = fopen("GnuPlot/CurrentSensity.gdt","w");
  fprintf(fp,"plot 'weidmann.dat' w p, %f w l, 'FD_Punkte.dat' w p pt 7 ps 3\n",FlowRate);
  fclose(fp);
  #endif

  free(SL); free(Df); free(FD[0]); free(FD[1]);
  return rho;
}


int DfMin(REAL *Df, int N, int ia, int ib)
{
  int i, idmin=ia;
  REAL dfmin=Df[ia];

  for(i=ia ; i<ib ; i++){
    if(Df[i]<dfmin){
      dfmin = Df[i];
      idmin = i;
    }
  }

  return idmin;
}
