int schnitt_ebene(REAL a1[3],REAL a2[3],REAL L,REAL sp[3])
{
  int out=0, k, l, ll;
  REAL y[3], dist[3], Ainv[3][3], det, eps=1.E-10;
  REAL a11=a1[0], a12=a1[1], a13=a1[2];
  REAL a21=a2[0], a22=a2[1], a23=a2[2];

  det = L*(a21 - a11 + a22 - a12) + a23 - a13;

  if(fabs(det)>eps){
    y[0]=-a11;
    y[1]=-a12;
    y[2]=L-a13;


    Ainv[0][0]=-(a23 - a13 + L*(a22 - a12))/det;
    Ainv[0][1]= (L*(a21 - a11))/det;
    Ainv[0][2]= (a21 - a11)/det;

    Ainv[1][0]= ((a22 - a12)*L)/det;
    Ainv[1][1]=-(a23 - a13 + L*(a21 - a11))/det;
    Ainv[1][2]= (a22 - a12)/det;
    
    Ainv[2][0]= L/det;
    Ainv[2][1]= L/det;
    Ainv[2][2]= 1./det;

    for(k=0 ; k<3 ; k++){
      sp[k] = 0.;
      for(l=0 ; l<3 ; l++)
	sp[k] += Ainv[k][l]*y[l];
    }

    dist[0] = 0.; dist[1] = 0.; dist[2] = 0.; 
    for(l=0 ; l<3 ; l++){
      dist[0] += pow(a1[l]-sp[l],2.);
      dist[1] += pow(a2[l]-sp[l],2.);
      dist[2] += pow(a1[l]-a2[l],2.);
    }
    for(l=0 ; l<3 ; l++)
      dist[l] = sqrt(dist[l]);
      
    printf("%f %f %f\n",sp[0],sp[1],sp[2]);
    printf("zwischen:\n");
    printf("%f %f %f\n",a1[0],a1[1],a1[2]);
    printf("%f %f %f\n\n",a2[0],a2[1],a2[2]);

    if(fabs(dist[0]+dist[1]-dist[2])<1.E-08)
      out = 1;

  } else {
    out = -1;
  }

  return out;
}

