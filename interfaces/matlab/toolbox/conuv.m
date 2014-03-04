function dydt = conuv(t, y, gas, mw)
% CONUV  Set up the ODE system for a constant-volume, adiabatic reactor.
% dydt = conuv(t, y, gas, mw)
% Function :mat:func:`conuv` evaluates the system of ordinary
% differential equations for an adiabatic, constant-volume,
% zero-dimensional reactor. It assumes that the 'gas' object
% represents a reacting ideal gas mixture.
%
% :param t:
%     Temperature
% :param y:
%     Vector of solution components. The order is:
%
%       - 1  - Temperature
%       - 2+ - Species mass fractions
%
% :param gas:
%     Ideal gas reacting mixture
% :param mw:
%     Vector of molecular weights of the species.
% :return:
%     Returns a vector of solution derivatives, size nSpecies + 1.
%

% Set the state of the gas, based on the current solution vector.
set(gas, 'T', y(1), 'Rho', density(gas), 'Y', y(2:end));
nsp = nSpecies(gas);

% energy equation
wdot = netProdRates(gas);
tdot = - temperature(gas) * gasconstant * (enthalpies_RT(gas) - ones(nsp,1))' ...
    * wdot / (density(gas)*cv_mass(gas));

% set up column vector for dydt
dydt = [ tdot
    zeros(nsp,1) ];

% species equations
rrho = 1.0/density(gas);
for i = 1:nsp
    dydt(i+1) = rrho*mw(i)*wdot(i);
end
