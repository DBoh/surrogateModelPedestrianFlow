REAL RhoEx(REAL x, REAL y, int ix, int iy);
REAL PhiEx(REAL x, REAL y, int ix, int iy);
void RHS_Helm(REAL *rhs);
REAL FDex(REAL x);
void HelpMatrixTest(char *, char *);
void MatrixIntTest(char *KOM, char *KOT, int FUNK[3]);
void MatrixTest(int flag);
void BoundMatrixTest(int flag);

void G_Cont_Exact_Vec(REAL , REAL *);
void G_Helm_Exact_Vec(REAL , REAL *);
void G_Helm_Exact_Entree_Vec(REAL , REAL *);
void HelmRHS(REAL time, REAL *rhs);

void Exact_Vec(REAL t, REAL *g, REAL (*func)(REAL, REAL , REAL));
REAL Rho_Exact(REAL t, REAL x, REAL y);
REAL dRho_Exact(REAL t, REAL x, REAL y, int l);
REAL d2Rho_Exact(REAL t, REAL x, REAL y, int ij);
void P_In_Exact_Vec(REAL , REAL *);
REAL U1_Exact(REAL t, REAL x, REAL y);
REAL dU1_Exact(REAL t, REAL x, REAL y, int flag);
REAL U2_Exact(REAL t, REAL x, REAL y);
REAL dU2_Exact(REAL t, REAL x, REAL y, int flag);
REAL dF_Exact(REAL t, REAL x, REAL y, int l);
REAL V_Exact(REAL t, REAL x, REAL y);
REAL dV_Exact_Vec(REAL t, REAL *g[2], REAL (*func)(REAL, REAL , REAL, int));
REAL dV_Exact(REAL t, REAL x, REAL y, int l);
REAL d2V_Exact(REAL t, REAL x, REAL y, int ij);
