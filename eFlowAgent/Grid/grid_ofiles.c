/*!
  \file grid_ofiles.c
*/
#include <ped.h>

extern int NK, NT, QuadN, QuadM;
extern GN *node;
extern GE *tri;
extern REAL *XY;
extern TRI refine;
extern REAL GridEpsZero, EpsZero;
extern QUADRA *quad;


//int search_T(REAL x, REAL y)
int search_tri(int k, REAL x, REAL y, int flag)
{
  int T, in=0, TT=-1, TTT=-1, ij, l, ii, jj, i, j;
  int imin, jmin, imax, jmax;
  int SearchArea=(int)(1./min(QuadN,QuadM)/refine.gridwidth)+1;
  int count=0;
  REAL xmmin, ymmin, xpmax, ypmax, xll, yll, xur, yur;

  /*
  TTT=search_T(k,x,y,flag);
  return TTT;
  */
  
  // unten links
  xmmin = x-refine.gridmax; ymmin = y-refine.gridmax;
  xll = max_REAL(0.,xmmin); yll = max_REAL(0.,ymmin);
  // oben rechts
  xpmax = x+refine.gridmax; ypmax = y+refine.gridmax;
  xur = min_REAL(1.,xpmax); yur = min_REAL(refine.boundy_ur,ypmax);
  
  // Suchbereich
  ij = X2Quad(xll,yll); // linke untere Ecke
  jmin = ij%QuadN; imin = (ij-jmin)/QuadN;
  ij = X2Quad(xur,yur); // rechte obere Ecke
  jmax = ij%QuadN; imax = (ij-jmax)/QuadN;
  
  ij = X2Quad(x,y);
  jj = ij%QuadN; ii = (ij-jj)/QuadN;
  
  for(i=imin ; i<=imax ; i++){
    for(j=jmin ; j<=jmax ; j++){
      ij = i*QuadN+j;
      for(l=0 ; l<quad[ij].Qnum ; l++){
	T = quad[ij].Q[l];
	in = inside(T,x,y);
	count++;
	if(in==1) {
	  TT = T;
	 
	  return TT;
	}
      }
    }
  }
  
  return TT;
}


int inside(int T, REAL x, REAL y)
{
  REAL xT[2], xd[2], eps=1.E-12; 

  xT[0] = x;
  xT[1] = y;

  transform_point(xT,T,xd);

  if(xd[0]>-eps && xd[0]<1.+eps && xd[1]>-eps && xd[1]<1.+eps 
     && xd[0]+xd[1]<1.+eps)
    return 1;

  return -1;
}


int inside_loc(REAL x1, REAL y1, REAL x2, REAL y2, REAL x3, REAL y3, REAL x, REAL y)
{
  REAL deti=0., detT;
  REAL xt[3], yt[3];
  int i, g, ins=0;

  xt[0] = x1;
  xt[1] = x2;
  xt[2] = x3;

  yt[0] = y1;
  yt[1] = y2;
  yt[2] = y3;

  detT = area_loc(x1,y1,x2,y2,x3,y3);
  for(i=0 ; i<3 ; i++) 
    deti += area_loc(xt[i],yt[i],x,y,xt[(i+1)%3],yt[(i+1)%3]);

  if(fabs(deti-detT) < 1.0e-12) ins = 1;

  return ins;

}


REAL area(int T)
{
  REAL xa=0., x1, y1, x2, y2, x3, y3;
  int i, i1, i2, i3;


  i1 = tri[T].VNR[0];
  i2 = tri[T].VNR[1];
  i3 = tri[T].VNR[2];
  x1 = XY[2*i1];
  y1 = XY[2*i1+1];
  x2 = XY[2*i2];
  y2 = XY[2*i2+1];
  x3 = XY[2*i3];
  y3 = XY[2*i3+1];
  xa = area_loc(x1,y1,x2,y2,x3,y3);
  return xa;

}


REAL area_loc(REAL x1, REAL y1,REAL x2, REAL y2,REAL x3, REAL y3)
{
  REAL xa=0.;

  xa = 0.5*fabs(determinante(x1-x2,y1-y2,x1-x3,y1-y3));
  return xa;

}


REAL determinante(REAL a1, REAL a2, REAL b1, REAL b2)
{
  return a1*b2-a2*b1;
}



int XY_to_SZ(REAL x, REAL y, int M, int N, int l)
{
  REAL a[2], xx[2];
  int Dim[2];

  a[0] = (1.*M)/(1.*N);
  a[1] = 1.;
  xx[0] = x;
  xx[1] = y;
  Dim[0] = N-1;
  Dim[1] = M-1;

  return (int)floor(xx[l]*Dim[l]*a[l]);

}

/*!						
  \brief length of the edge in the opposit od i
*/
REAL tri_length(int k, int i)
{
  int i1, i2;
  REAL x1, x2, y1, y2;

  i1 = tri[k].VNR[(i+1)%3];
  i2 = tri[k].VNR[(i+2)%3];

  x1 = XY[2*i1];
  y1 = XY[2*i1+1];

  x2 = XY[2*i2];
  y2 = XY[2*i2+1];

  return sqrt(pow(x1-x2,2.)+pow(y1-y2,2.));
}

REAL gridwidth()
{
  int k;
  REAL h, gw=0.;

  if(refine.ll){
    gw = sqrt(1./NK);
  } else {
    for(k=0 ; k<NT ; k++){
      h = gridwidth_loc(k);
      if(h>gw) gw=h;
    }
  }
  return gw;
}

REAL MeanGridWidth()
{
  int k;
  REAL h=0.;

  for(k=0 ; k<NT ; k++)
    h += gridwidth_loc(k);

  return h/(REAL)NT;
}

REAL gridwidth_loc(int k)
{
  int l;
  REAL max=0., le[3];

  gridwidth_edge(k,le);

  for(l=0 ; l<3 ; l++)
    if(le[l]>max) max=le[l];

  return max;
}

void gridwidth_edge(int k, REAL le[3])
{
  int l, i1, i2;
  REAL x1, y1, x2, y2, max=0., xl;

  for(l=0 ; l<3 ; l++){
    i1 = tri[k].VNR[l];
    x1 = XY[2*i1]; y1 = XY[2*i1+1];
    i2 = tri[k].VNR[(l+1)%3];
    x2 = XY[2*i2]; y2 = XY[2*i2+1];
    le[l] = sqrt(pow(x2-x1,2.)+pow(y2-y1,2.));
  }
}

void GridwidthMinMax(REAL minmax[2])
{
  int k;
  REAL det;

  minmax[0]=detk(0);
  minmax[1]=minmax[0];

  for(k=1 ; k<NT ; k++){
    det = detk(k);
    if(det<minmax[0]) minmax[0]=det;
    if(det>minmax[1]) minmax[1]=det;
  }
}

/*!
  \brief search triangle that includes (x1,x2)
*/
//int search_tri(int k, REAL x1, REAL x2, int flag)
int search_T(int k, REAL x, REAL y, int flag)
{
  REAL x1, x2, x3, y1, y2, y3;
  for(k=0 ; k<NT ; k++){
    x1 = XY[2*tri[k].VNR[0]]; y1 = XY[2*tri[k].VNR[0]+1];
    x2 = XY[2*tri[k].VNR[1]]; y2 = XY[2*tri[k].VNR[1]+1];
    x3 = XY[2*tri[k].VNR[2]]; y3 = XY[2*tri[k].VNR[2]+1];
    if(inside_loc(x1,y1,x2,y2,x3,y3,x,y)) return k;
  }
  return -1;
}


int search_T_Neigh(int k, REAL x1, REAL x2, int flag)
{
  int l, kl, kk;
  REAL x[2], xd[2], eps=GridEpsZero;
  REAL f1, f2, f3;

  x[0] = x1;
  x[1] = x2;

  transform_point(x,k,xd);

  if(xd[0]>-eps && xd[0]<1.+eps && xd[1]>-eps && xd[1]<1.+eps 
     && xd[0]+xd[1]<1.+eps)
    return k;

  
  for(l=0 ; l<3 ; l++){
    kl = tri[k].NEIGH[l];
    if(kl>-1){
      transform_point(x,kl,xd);
      if(xd[0]>-eps && xd[0]<1.+eps && xd[1]>-eps && xd[1]<1.+eps 
	 && xd[0]+xd[1]<1.+eps)
	return kl;
    }
  }

  for(kk=0 ; kk<NT ; kk++){
    transform_point(x,kk,xd);
    if(xd[0]>-eps && xd[0]<1.+eps && xd[1]>-eps && xd[1]<1.+eps 
       && xd[0]+xd[1]<1.+eps)
      return kk;
  }

  if(flag){
    printf("GridEpsZero = %e\n",GridEpsZero);
    printf("E: No simplex found <seach_tri>\n");
    printf("(x,y)=(%lf,%lf)\n",x1,x2);
  }

  return -1;

}

/*
inverse Matrix zur Abbildung Phi: Td nach T+k
*/
void Akinv(REAL mmat[2][2], int k)
{
  REAL xx[3], yy[3], det;
  int l;

  for(l=0 ; l<3 ; l++){
    xx[l] = XY[2*tri[k].VNR[l]];
    yy[l] = XY[2*tri[k].VNR[l]+1];
  }

  det = (xx[1]-xx[0])*(yy[2]-yy[0])-(xx[0]-xx[2])*(yy[0]-yy[1]);
  mmat[0][0] = (yy[2]-yy[0])/det;
  mmat[0][1] = (xx[0]-xx[2])/det;
  mmat[1][0] = (yy[0]-yy[1])/det;
  mmat[1][1] = (xx[1]-xx[0])/det;

}

REAL detk(int k)
{
  int g1, g2, g3;
  REAL det=0., mmat[2][2];

  g1 = tri[k].VNR[0];
  g2 = tri[k].VNR[1];
  g3 = tri[k].VNR[2];
  
  mmat[0][0] = XY[2*g3+1] - XY[2*g1+1];
  mmat[0][1] = XY[2*g1] - XY[2*g3];
  mmat[1][0] = XY[2*g1+1] - XY[2*g2+1];
  mmat[1][1] = XY[2*g2] - XY[2*g1];
    
  det = mmat[1][1]*mmat[0][0]-mmat[0][1]*mmat[1][0];
  
  return fabs(det);
}


void BoundaryNormal(REAL *nu[2])
{
  int l, k, j, j1, j2;
  REAL nuk[2], bn;

  for(l=0 ; l<2 ; l++)
    field_cp(nu[l],0.,nu[l],0.,nu[l],0.,NK);


  for(k=0 ; k<NT ; k++){
    for(j=0 ; j<3 ; j++){
      if(tri[k].NEIGH[j]<0){
	j1 = tri[k].VNR[(j+1)%3];
	j2 = tri[k].VNR[(j+2)%3];
	
	nuk[0] = XY[2*j2+1]-XY[2*j1+1];
	nuk[1] = XY[2*j1]-XY[2*j2];

	for(l=0 ; l<2 ; l++){
	  nu[l][j1] += nuk[l];
	  nu[l][j2] += nuk[l];
	}

      }
    }
  }
  
  for(j=0 ; j<NK ; j++){
    if(node[j].id<0){
      for(l=0 ; l<2 ; l++)
	nuk[l] = nu[l][j];
      bn = sqrt(pow(nuk[0],2.)+pow(nuk[1],2.));
      if(fabs(bn)>=1.e-12){
	//	ErrorMessage("Nu = zero","BoundaryNormal");
	for(l=0 ; l<2 ; l++)
	  nu[l][j] /= bn;
      }
    }
  }
}

void WallNormal()
{
  int l, k, j, j1, j2;
  REAL nuk[2], bn;

  for(j=0 ; j<NK ; j++)
    for(l=0 ; l<2 ; l++){
      node[j].WallNu[l] = 0.;
      node[j].EntreeNu[l] = 0.;
      nuk[l] = 0;
    }


  
  for(k=0 ; k<NT ; k++){
    for(j=0 ; j<3 ; j++){
      if(tri[k].NEIGH[j]<0){
	j1 = tri[k].VNR[(j+1)%3];
	j2 = tri[k].VNR[(j+2)%3];
	
	nuk[0] = XY[2*j2+1]-XY[2*j1+1];
	nuk[1] = XY[2*j1]-XY[2*j2];

	for(l=0 ; l<2 ; l++){
	  node[j1].WallNu[l] += nuk[l];
	  node[j2].WallNu[l] += nuk[l];
	}

      }
    }
  }
  
  for(j=0 ; j<NK ; j++){
    if(node[j].id<0){
      for(l=0 ; l<2 ; l++)
	nuk[l] = node[j].WallNu[l];
      bn = sqrt(pow(nuk[0],2.)+pow(nuk[1],2.));
      if(fabs(bn)>=1.e-12){
	for(l=0 ; l<2 ; l++)
	  node[j].WallNu[l] /= bn;
      }
      
      if(ChkEntree(j))
	for(l=0 ; l<2 ; l++)
	  node[j].EntreeNu[l] = node[j].WallNu[l];
      
      
    }
  }

#if PLOT==2
  REAL *vel[2];
  for(l=0 ; l<2 ; l++)
    vel[l] = (REAL *)calloc(NK , sizeof(REAL ));

  for(j=0 ; j<NK ; j++)
    for(l=0 ; l<2 ; l++)
      vel[l][j] = node[j].WallNu[l];
  octaveplotvec(vel,"WallNu",0);
  for(l=0 ; l<2 ; l++)
    free(vel[l]);
#endif
}

int BoundEdgeTri(int k)
{
    int l;
    for(l=0 ; l<3 ; l++)
        if(tri[k].NEIGH[l]<0)
            return -(l+1);
    return 0;
}

int BoundNodeTri(int k)
{
    int l;
    for(l=0 ; l<3 ; l++)
        if(node[tri[k].VNR[l]].id<0) return -(l+1);
    return 0;
}
