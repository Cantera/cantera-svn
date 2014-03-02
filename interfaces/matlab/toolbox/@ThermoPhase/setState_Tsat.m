function setState_Tsat(tp,tx)
% Set the fluid to a saturated state at temperature tx
%
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     class derived from ThermoPhase)
% :param tx:
%     Temperature. Units: K
%

thermo_set(tp.tp_id,25,tx);
