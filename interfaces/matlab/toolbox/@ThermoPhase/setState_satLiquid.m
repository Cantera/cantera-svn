function setState_satLiquid(tp)
% Set the fluid to the saturated liquid state at the current temperature.
%
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     class derived from ThermoPhase)
%

thermo_set(tp.tp_id,2,0);
