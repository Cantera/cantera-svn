function v = satPressure(tp, T)
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     object that derives from ThermoPhase)
% :param T:
%     Temperature in K
% :return:
%     Saturation pressure for temperature T. Units: Pa
%

v = thermo_get(tp.tp_id,24,T);
