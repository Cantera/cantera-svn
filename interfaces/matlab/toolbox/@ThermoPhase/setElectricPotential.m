function setElectricPotential(tp,phi)
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     class derived from ThermoPhase)
% :param phi:
%     Electric potential. Units: V
%

thermo_set(tp.tp_id,2,phi);
