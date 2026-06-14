clear all
close all

syms x y

uvecx=1; uvecy=1;

uu=[1, x+y, (x+y)^2, sin(x)*cos(y)];

for i=1:4
    for j=1:4
        for k=1:4
            u1=uu(i); u2=uu(j); u3=uu(k);

% Mass
%           v = u1*u2*u3;
            
% Stiff
             dux = diff(u1,x); duy = diff(u1,y);
             dvx = diff(u2,x); dvy = diff(u2,y);
             v=(dux*dvx+duy*dvy)*u3;
% Konv with onevec
%              dvx = diff(u2,x); dvy = diff(u2,y); 
%              v = (dvx*u3+dvy*u3)*u1;
             U=int(int(v,x,0,1),y,0,1);
             fprintf("    IntEx[%d][%d][%d]=%.10e;\n",i-1,j-1,k-1,U);
        end
%        fprintf("\n");
    end
end





%a=0; b=1;
%I0 = subs(subs(U,x,b),y,b)-subs(subs(U,x,a),y,a)