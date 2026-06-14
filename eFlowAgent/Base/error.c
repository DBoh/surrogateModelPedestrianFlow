/*!
  \file error.c

  \brief functions to compute L2- and H1-discretisation errors.
*/

#include <ped.h>

extern QUADRATUR QUAD;
extern int NT;
extern Basis phi;
extern TRI refine;


void ErrorVecFunc(REAL t, REAL *u[2], REAL (*f1)(REAL t, REAL x, REAL y), REAL (*f2)(REAL t, REAL x, REAL y), REAL (*df1)(REAL t, REAL x, REAL y, int flag), REAL (*df2)(REAL t, REAL x, REAL y, int flag), char *bla)
{
  REAL l2e1, l2e2, h1e1, h1e2, l2e, h1e, h;
  char msg1[100], msg2[100];

  l2e1 = L2ErrorFunc(t,u[0],f1);
  l2e2 = L2ErrorFunc(t,u[1],f2);
  l2e = sqrt(l2e1*l2e1+l2e2*l2e2);
  h1e1 = H1ErrorFunc(t,u[0],df1);
  h1e2 = H1ErrorFunc(t,u[1],df2);
  h1e = sqrt(h1e1*h1e1+h1e2*h1e2);
  h   = refine.gridwidth;

  #if MSG==1
  sprintf(msg1,"%s: Error at time: %6.2f ",bla,t);
  sprintf(msg2," %.2e %.2e %.2e",h,l2e,h1e);
  Message(msg1,'.',msg2);
  #endif
}


/*!
  \brief L2- and H1-discretisation error
 */
void ErrorFunc(REAL t, REAL *u, REAL (*f)(REAL t, REAL x, REAL y), REAL (*df)(REAL t, REAL x, REAL y, int flag), char *bla)
{
  REAL l2e, h1e, h;
  char msg1[100], msg2[100];

  l2e = L2ErrorFunc(t,u,f);
  h1e = H1ErrorFunc(t,u,df);
  h   = refine.gridwidth;

  #if MSG==1
  sprintf(msg1,"%s: Error at time: %6.2f ",bla,t);
  sprintf(msg2," %.2e %.2e %.2e",h,l2e,h1e);
  Message(msg1,'.',msg2);
  #endif
}

/*!
  \brief L2-error: \f$\|u-u_h\|_{L^2}=\f$
  \f[\left(\int\limits_{\Omega_h}|u- u_h|^2~dx\right)^\frac{1}{2}\f]
*/
REAL L2ErrorFunc(REAL t, REAL *u, REAL (*f)(REAL t, REAL x, REAL y))
{
  int k, m, mm, l;
  REAL xd[2], x[2], st[3]={1./6.,2./3.,1./6.};
  REAL u_e, u_h, sum, det, l2err;

  l2err=0.;
  for(k=0 ; k<NT ; k++){
    det = detk(k);
    sum=0.;
    for(m=0 ; m<3 ; m++){
      mm=2-m;
      xd[0] = st[mm%3];
      xd[1] = st[(mm+1)%3];
      transform2_point(xd,k,x);
      u_e = f(t,x[0],x[1]);
      u_h = uh(k,x[0],x[1],u);
      sum += pow(u_e-u_h,2.)*QUAD.w_2d[m];
    }
    sum *= det;
    l2err += sum;
  }

  l2err = sqrt(l2err);
  return l2err;

}

/*!
  \brief H1-error  \f$|u-u_h|_{H^1}=\|\nabla(u-u_h)\|_{L^2}=\f$
  \f[\left(\int\limits_{\Omega_h}|\nabla u-\nabla u_h|^2~dx\right)^\frac{1}{2}\f]
*/
REAL H1ErrorFunc(REAL t, REAL *u, REAL (*df)(REAL t, REAL x, REAL y, int flag))
{
  int k, m, mm, l;
  REAL xd[2], x[2], st[3]={1./6.,2./3.,1./6.};
  REAL du_e[2], du_h[2], sum, det, h1err, sumd;

  h1err=0.;
  for(k=0 ; k<NT ; k++){
    det = detk(k);
    sum=0.;
    for(m=0 ; m<3 ; m++){
      mm=2-m;
      xd[0] = st[mm%3];
      xd[1] = st[(mm+1)%3];
      transform2_point(xd,k,x);

      for(l=0 ; l<2 ; l++)
	du_e[l] = df(t,x[0],x[1],l);

      duh(du_h,k,u);
      sumd = pow(du_e[0]-du_h[0],2)+pow(du_e[1]-du_h[1],2);

      sum += sumd*QUAD.w_2d[m];
    }
    h1err += sum*det;
  }

  h1err = sqrt(h1err);
  return h1err;

}



