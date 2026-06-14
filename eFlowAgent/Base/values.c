#include <structdef.h>

extern INFO output;
extern QUADRATUR QUAD;
extern Basis    phi;
extern DAT      PED;
extern TRI      refine;
extern SOLV     solver;
extern System   lgs;
extern INFOS    msg;
extern int      bandwidth;
extern int      EikonalMode;

void standardvalues()
{
  int l;
  
  // standardvalues:
  output.KOI=4;
  output.IL=2;
  output.edge=0;
  output.corr=0;

  QUAD.st_1d[0]=0.211324865405187;
  QUAD.st_1d[1]=0.788675134594813;
  QUAD.w_1d[0]=0.5;
  QUAD.w_1d[1]=0.5;

  QUAD.st_2d[0][0]=1./6.;
  QUAD.st_2d[0][1]=2./3.;
  QUAD.st_2d[1][0]=2./3.;
  QUAD.st_2d[1][1]=1./6.;
  QUAD.st_2d[2][0]=1./6.;
  QUAD.st_2d[2][1]=1./6.;
  QUAD.w_2d[0]=1./6.;
  QUAD.w_2d[1]=1./6.;
  QUAD.w_2d[2]=1./6.;

  phi.DEG=1;
  PED.MassMax=1.0e-12;

  EikonalMode = 0;
  
  /********************************************************************/
  /* ehemals pDevelop.dat

  # Fundamentaldiagramm
  3		# FD (0: 1-r, 1: afac, 2: 1-exp(...), 3: Goatin, 4: weidmann
  00Input/weidmann.dat  # Weidmann-data
  # discretisation _____________________________________________________
  0.1		 # PED.delta (time)
  0		 # globale Verfeinerung
  0       	 # lokale  Verfeinerung
  0       	 # Modus
  # regularities _______________________________________________________
  -0.001	       # PED.EPS0	
  -0.1	       # PED.EPS1	
  1e-08	       # PED.EPS2	
  # characteristics ____________________________________________________
  0.45		  # PED.Shoulder (width of human shoulder)
  1		  # PED.empty (lower limit in %of MaxPeople)
  1       	  # PED.CT  ( char. Zeit    in sec          )
  # numerics: solver ___________________________________________________
  2           # solver.mode: mode 1 (CG), 2 (PBICGSTAB)		
  1000  	    # olver.itmax
  1.E-12	    # solver.akrit
  1	    # solver.precon
  # numerics: LGS ______________________________________________________
  1.E-0	    # lgs.alpha
  1.E-0	    # lgs.beta		
  1.E-0	    # lgs.gamma
  1.E-01	    # lgs.delta
  4	    # lgs.bound
  #Some security applications __________________________________________
  1     	       # Warnings
  0	       # Checks
  1	       # Errors
  */

  PED.delta=0.1;
  refine.lg=0;
  refine.ll=0;
  refine.mode=0;
  PED.EPS0=-0.1;
  PED.EPS1=0.01; //0.4;
  PED.EPS2=1e-08;
  PED.Shoulder=0.45;
  PED.empty=1;
  PED.CT=1;
  solver.mode=2;   
  solver.itmax=1000;
  solver.akrit=1e-08;
  solver.precon=1;
  lgs.alpha=1;
  lgs.beta=1;
  lgs.gamma=1;
  lgs.delta=0.1;
  lgs.bound=4;
  msg.Warning=1;
  msg.Check=0;
  msg.Error=1;

  PED.plotfreq=10;

  bandwidth=11;


  /* Weidmann */
  for(l=0 ; l<2 ; l++)
    PED.FundamentalDiagram[l] = (REAL *)calloc(91 , sizeof(REAL ));


  PED.FundamentalDiagram[0][0]=0.001268; PED.FundamentalDiagram[1][0]=1.344080;
  PED.FundamentalDiagram[0][1]=0.072971; PED.FundamentalDiagram[1][1]=1.331990;
  PED.FundamentalDiagram[0][2]=0.136691; PED.FundamentalDiagram[1][2]=1.321910;
  PED.FundamentalDiagram[0][3]=0.204516; PED.FundamentalDiagram[1][3]=1.305840;
  PED.FundamentalDiagram[0][4]=0.260300; PED.FundamentalDiagram[1][4]=1.295770;
  PED.FundamentalDiagram[0][5]=0.320143; PED.FundamentalDiagram[1][5]=1.281700;
  PED.FundamentalDiagram[0][6]=0.387968; PED.FundamentalDiagram[1][6]=1.265630;
  PED.FundamentalDiagram[0][7]=0.443933; PED.FundamentalDiagram[1][7]=1.247570;
  PED.FundamentalDiagram[0][8]=0.503821; PED.FundamentalDiagram[1][8]=1.231510;
  PED.FundamentalDiagram[0][9]=0.555772; PED.FundamentalDiagram[1][9]=1.215460;
  PED.FundamentalDiagram[0][10]=0.611737; PED.FundamentalDiagram[1][10]=1.197400;
  PED.FundamentalDiagram[0][11]=0.671806; PED.FundamentalDiagram[1][11]=1.173350;
  PED.FundamentalDiagram[0][12]=0.719834; PED.FundamentalDiagram[1][12]=1.155300;
  PED.FundamentalDiagram[0][13]=0.763894; PED.FundamentalDiagram[1][13]=1.137260;
  PED.FundamentalDiagram[0][14]=0.799972; PED.FundamentalDiagram[1][14]=1.121230;
  PED.FundamentalDiagram[0][15]=0.847955; PED.FundamentalDiagram[1][15]=1.105190;
  PED.FundamentalDiagram[0][16]=0.884169; PED.FundamentalDiagram[1][16]=1.083170;
  PED.FundamentalDiagram[0][17]=0.928183; PED.FundamentalDiagram[1][17]=1.067130;
  PED.FundamentalDiagram[0][18]=0.964442; PED.FundamentalDiagram[1][18]=1.043110;
  PED.FundamentalDiagram[0][19]=1.004440; PED.FundamentalDiagram[1][19]=1.029070;
  PED.FundamentalDiagram[0][20]=1.040610; PED.FundamentalDiagram[1][20]=1.009040;
  PED.FundamentalDiagram[0][21]=1.076820; PED.FundamentalDiagram[1][21]=0.987024;
  PED.FundamentalDiagram[0][22]=1.120980; PED.FundamentalDiagram[1][22]=0.964991;
  PED.FundamentalDiagram[0][23]=1.157320; PED.FundamentalDiagram[1][23]=0.936978;
  PED.FundamentalDiagram[0][24]=1.193450; PED.FundamentalDiagram[1][24]=0.918951;
  PED.FundamentalDiagram[0][25]=1.225510; PED.FundamentalDiagram[1][25]=0.904925;
  PED.FundamentalDiagram[0][26]=1.261680; PED.FundamentalDiagram[1][26]=0.884901;
  PED.FundamentalDiagram[0][27]=1.305920; PED.FundamentalDiagram[1][27]=0.858874;
  PED.FundamentalDiagram[0][28]=1.350160; PED.FundamentalDiagram[1][28]=0.832846;
  PED.FundamentalDiagram[0][29]=1.402200; PED.FundamentalDiagram[1][29]=0.812799;
  PED.FundamentalDiagram[0][30]=1.446350; PED.FundamentalDiagram[1][30]=0.790767;
  PED.FundamentalDiagram[0][31]=1.474540; PED.FundamentalDiagram[1][31]=0.772751;
  PED.FundamentalDiagram[0][32]=1.510710; PED.FundamentalDiagram[1][32]=0.752727;
  PED.FundamentalDiagram[0][33]=1.554720; PED.FundamentalDiagram[1][33]=0.736686;
  PED.FundamentalDiagram[0][34]=1.582860; PED.FundamentalDiagram[1][34]=0.720668;
  PED.FundamentalDiagram[0][35]=1.623000; PED.FundamentalDiagram[1][35]=0.700638;
  PED.FundamentalDiagram[0][36]=1.675090; PED.FundamentalDiagram[1][36]=0.678594;
  PED.FundamentalDiagram[0][37]=1.723120; PED.FundamentalDiagram[1][37]=0.660550;
  PED.FundamentalDiagram[0][38]=1.771330; PED.FundamentalDiagram[1][38]=0.634517;
  PED.FundamentalDiagram[0][39]=1.827290; PED.FundamentalDiagram[1][39]=0.616461;
  PED.FundamentalDiagram[0][40]=1.883390; PED.FundamentalDiagram[1][40]=0.592414;
  PED.FundamentalDiagram[0][41]=1.935390; PED.FundamentalDiagram[1][41]=0.574364;
  PED.FundamentalDiagram[0][42]=1.979450; PED.FundamentalDiagram[1][42]=0.556326;
  PED.FundamentalDiagram[0][43]=2.035410; PED.FundamentalDiagram[1][43]=0.538270;
  PED.FundamentalDiagram[0][44]=2.075410; PED.FundamentalDiagram[1][44]=0.524232;
  PED.FundamentalDiagram[0][45]=2.123350; PED.FundamentalDiagram[1][45]=0.510183;
  PED.FundamentalDiagram[0][46]=2.167370; PED.FundamentalDiagram[1][46]=0.494142;
  PED.FundamentalDiagram[0][47]=2.227350; PED.FundamentalDiagram[1][47]=0.474083;
  PED.FundamentalDiagram[0][48]=2.283310; PED.FundamentalDiagram[1][48]=0.456027;
  PED.FundamentalDiagram[0][49]=2.331200; PED.FundamentalDiagram[1][49]=0.443975;
  PED.FundamentalDiagram[0][50]=2.383060; PED.FundamentalDiagram[1][50]=0.431917;
  PED.FundamentalDiagram[0][51]=2.442950; PED.FundamentalDiagram[1][51]=0.415853;
  PED.FundamentalDiagram[0][52]=2.514790; PED.FundamentalDiagram[1][52]=0.397774;
  PED.FundamentalDiagram[0][53]=2.574630; PED.FundamentalDiagram[1][53]=0.383707;
  PED.FundamentalDiagram[0][54]=2.634430; PED.FundamentalDiagram[1][54]=0.371637;
  PED.FundamentalDiagram[0][55]=2.706180; PED.FundamentalDiagram[1][55]=0.357553;
  PED.FundamentalDiagram[0][56]=2.777840; PED.FundamentalDiagram[1][56]=0.347463;
  PED.FundamentalDiagram[0][57]=2.865460; PED.FundamentalDiagram[1][57]=0.333356;
  PED.FundamentalDiagram[0][58]=2.929220; PED.FundamentalDiagram[1][58]=0.321280;
  PED.FundamentalDiagram[0][59]=3.000880; PED.FundamentalDiagram[1][59]=0.311191;
  PED.FundamentalDiagram[0][60]=3.072490; PED.FundamentalDiagram[1][60]=0.303098;
  PED.FundamentalDiagram[0][61]=3.152180; PED.FundamentalDiagram[1][61]=0.289002;
  PED.FundamentalDiagram[0][62]=3.227760; PED.FundamentalDiagram[1][62]=0.280904;
  PED.FundamentalDiagram[0][63]=3.303380; PED.FundamentalDiagram[1][63]=0.270808;
  PED.FundamentalDiagram[0][64]=3.367010; PED.FundamentalDiagram[1][64]=0.264724;
  PED.FundamentalDiagram[0][65]=3.426770; PED.FundamentalDiagram[1][65]=0.254652;
  PED.FundamentalDiagram[0][66]=3.502300; PED.FundamentalDiagram[1][66]=0.248551;
  PED.FundamentalDiagram[0][67]=3.573960; PED.FundamentalDiagram[1][67]=0.238461;
  PED.FundamentalDiagram[0][68]=3.645570; PED.FundamentalDiagram[1][68]=0.230368;
  PED.FundamentalDiagram[0][69]=3.721110; PED.FundamentalDiagram[1][69]=0.224267;
  PED.FundamentalDiagram[0][70]=3.792720; PED.FundamentalDiagram[1][70]=0.216174;
  PED.FundamentalDiagram[0][71]=3.892020; PED.FundamentalDiagram[1][71]=0.212036;
  PED.FundamentalDiagram[0][72]=3.963580; PED.FundamentalDiagram[1][72]=0.205941;
  PED.FundamentalDiagram[0][73]=4.031180; PED.FundamentalDiagram[1][73]=0.199851;
  PED.FundamentalDiagram[0][74]=4.122590; PED.FundamentalDiagram[1][74]=0.193727;
  PED.FundamentalDiagram[0][75]=4.206060; PED.FundamentalDiagram[1][75]=0.187614;
  PED.FundamentalDiagram[0][76]=4.301440; PED.FundamentalDiagram[1][76]=0.181484;
  PED.FundamentalDiagram[0][77]=4.392890; PED.FundamentalDiagram[1][77]=0.173363;
  PED.FundamentalDiagram[0][78]=4.488230; PED.FundamentalDiagram[1][78]=0.169230;
  PED.FundamentalDiagram[0][79]=4.591590; PED.FundamentalDiagram[1][79]=0.161091;
  PED.FundamentalDiagram[0][80]=4.678980; PED.FundamentalDiagram[1][80]=0.156970;
  PED.FundamentalDiagram[0][81]=4.778420; PED.FundamentalDiagram[1][81]=0.146840;
  PED.FundamentalDiagram[0][82]=4.865950; PED.FundamentalDiagram[1][82]=0.136727;
  PED.FundamentalDiagram[0][83]=4.961410; PED.FundamentalDiagram[1][83]=0.126603;
  PED.FundamentalDiagram[0][84]=5.056930; PED.FundamentalDiagram[1][84]=0.114481;
  PED.FundamentalDiagram[0][85]=5.132640; PED.FundamentalDiagram[1][85]=0.100391;
  PED.FundamentalDiagram[0][86]=5.200420; PED.FundamentalDiagram[1][86]=0.086312;
  PED.FundamentalDiagram[0][87]=5.276280; PED.FundamentalDiagram[1][87]=0.066231;
  PED.FundamentalDiagram[0][88]=5.324210; PED.FundamentalDiagram[1][88]=0.052181;
  PED.FundamentalDiagram[0][89]=5.368230; PED.FundamentalDiagram[1][89]=0.036140;
  PED.FundamentalDiagram[0][90]=5.400470; PED.FundamentalDiagram[1][90]=0.000000;
  
}


void dimensionalisation()
{


}

