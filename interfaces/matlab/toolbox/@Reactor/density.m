function rho = density(r)
% Get the density of the phase in the input reactor.
%
% :param r:
%     Instance of class :mat:func:`Reactor`
% :return:
%     Density of the phase in the input. Units: kg/m**3
%

rho = reactormethods(25, reactor_hndl(r));
