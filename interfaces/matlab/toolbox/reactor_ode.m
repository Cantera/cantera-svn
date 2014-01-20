function dydt = reactor_ode(t,y,gas,vdot,area,heatflux)
% ODE system for a generic zero-dimensional reactor.
%
% Function :mat:func:`reactor_ode` evaluates the system of ordinary
% differential equations for a zero-dimensional reactor with arbitrary
% heat transfer and  volume change.
%
% Solution vector components:
%     - y(1)   Total internal energy U
%     - y(2)   Volume V
%     - y(3)   Mass of species 1
%     - ⁞
%     - y(2+nsp) Mass of last species
%
% :param t:
%     Reactor temperature
% :param y:
%     Solution vector
% :param gas:
%     Instance of class :mat:func:`Solution` representing the contents
%     of the reactor.
% :param vdot:
%     Rate of change of the reactor volume.
% :param area:
%     Surface area of the reactor undergoing heat transfer.
% :param heatflux:
%     Heat flux into or out of the reactor.
% :return:
%     Vector of length nSpecies + 2 representing the solution 
%     derivative.
%

[m,n] = size(y);
dydt = zeros(m,n);

for j = 1:n
    this_y = y(:,j);
    int_energy = this_y(1);
    vol = this_y(2);
    masses = this_y(3:end);

    % evaluate the total mass, and the specific internal energy and volume.
    total_mass = sum(masses);
    u_mass = int_energy/total_mass;
    v_mass = vol/total_mass;

    % set the state of the gas by specifying (u,v,{Y_k})
    setMassFractions(gas,masses);
    setState_UV(gas, [u_mass v_mass]);
    p = pressure(gas);

    % volume equation
    vdt = feval(vdot, t, vol, gas);

    % energy equation
    a = feval(area, t, vol);
    q = feval(heatflux, t, gas);
    udt = -p * vdt + a * q;

    % species equations
    ydt = total_mass * ydot(gas);

    % set up column vector for dydt
    dydt(:,j) = [udt
        vdt
        ydt ];
end
