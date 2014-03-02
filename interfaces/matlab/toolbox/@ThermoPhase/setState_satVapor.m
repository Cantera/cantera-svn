function setState_satVapor(tp)
% Set the fluid to the saturated vapor state at the current temperature.
%
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     class derived from ThermoPhase)
%

thermo_set(a.tp_id,3,0);
