clear all
close all


Q=load("test.dat");
Q(end+1,:) = Q(1,:);
Pmin = min(Q);
Pmax = max(Q);


P=scaldown(Q,2,Pmin,Pmax);

Q2=scalup(P,2,Pmin,Pmax);


plot(Q(:,1),Q(:,2),'r-')
grid on
hold on
plot(P(:,1),P(:,2),'b-')
plot(Q2(:,1),Q2(:,2),'ro')


function P=scaldown(Q,a,Pmin,Pmax)

P = (Q-Pmin)*a/(Pmax(1)-Pmin(1));

end

function P=scalup(Q,a,Pmin,Pmax)

P = Q*(Pmax(1)-Pmin(1))/a+Pmin;

end



