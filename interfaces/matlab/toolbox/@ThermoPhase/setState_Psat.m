function setState_Psat(tp,px)
% Set the fluid to a saturated state at pressure px
%
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     class derived from ThermoPhase)
% :param px:
%     Pressure. Units: Pa
%
thermo_set(tp.tp_id,24,px);
