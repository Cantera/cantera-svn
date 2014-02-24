function b = soundspeed(a)
% Speed of sound (m/s).
%
% :param a:
%     Instance of class :mat:func:`Solution`
% :return:
%     The speed of sound in m/s
%

if isIdealGas(a)
    gamma = cp_mass(a)/cv_mass(a);
    wtm = meanMolecularWeight(a);
    r = gasconstant/wtm;
    b = sqrt(gamma * r * temperature(a));
else
    rho0 = density(a);
    p0 = pressure(a);
    s0 = entropy_mass(a);
    rho1 = 1.001*rho0;
    set(gas,'Density',rho1,'Entropy',s0);
    p1 = pressure(a);
    dpdrho_s = (p1 - p0)/(rho1 - rho0);
    b = sqrt(dpdrho_s);
end
