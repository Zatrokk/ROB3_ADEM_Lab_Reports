%   PWM  Omega  Tau
M = [885  297  -10;
     885  0    2000; 
     885  150  1000; 
     442  150  -10; 
     442  0    1200; 
     442  75   500; 
     221  75   -10; 
     221  0    600; 
     221  37   250; 
     110  38   -50; 
     110  0    290; 
     110  19   120; 
     0    0    0];

%    Tau     OMEGA
A = [M(:,3) M(:,2)];

%    PWM
B = M(:,1);

C = A\B;

s = 13;
tau_calc = [s];


for i = 1:s
    tau_calc(i) = (1/0.4214)*(B(i)-A(i,2)*3.0156)
end

%scatter(tau_calc,A(:,1))
%lsline

figure(1)
scatter(tau_calc,A(:,1))
grid on;
xlabel('Calculated torque');
ylabel('Measured torque');
title('Comparison of measured and calculated torque');
hl = lsline;
D = [ones(size(hl.XData(:))), hl.XData(:)]\hl.YData(:);
Slope = D(2)
Intercept = D(1)