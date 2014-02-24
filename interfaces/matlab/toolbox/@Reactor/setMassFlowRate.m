function setMassFlowRate(r, mdot)
% Set the mass flow rate.
%
% :param r:
%     Instance of class :mat:func:`Reactor`
% :param mdot:
%     Mass flow rate. Units: kg/s
%

reactormethods(10, reactor_hndl(r), mdot);
