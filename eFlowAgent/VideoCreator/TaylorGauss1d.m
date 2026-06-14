clear all
close all

syms x beta gamma

I=[-2,2];
DIM=100;
D = 6;
x0 = beta;

%%
g(x) = 1/sqrt(gamma*2*pi)*(exp(-(x-beta).^2/gamma^2));

dg(x) = g(x);

Tg(x) = subs(dg(x),x,x0);
for i=1:D
    dg(x) = diff(dg(x),x);
    dTg = subs(dg(x),x,x0);
    Tg(x) = Tg(x) + dTg/factorial(i)*(x-x0).^i;
end

xx=linspace(I(1),I(2),DIM);
yy=subs(subs(Tg(xx),beta,0),gamma,1);
gg=subs(subs(g(xx),beta,0),gamma,1);

plot(xx,yy)
hold on
grid on
plot(xx,gg)
