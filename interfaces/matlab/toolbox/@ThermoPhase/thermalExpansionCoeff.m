function a = thermalExpansionCoeff(tp)
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     class derived from ThermoPhase)
% :return:
%     Thermal Expansion Coefficient. Units: 1/K
%

a = thermo_get(tp.tp_id,27);
