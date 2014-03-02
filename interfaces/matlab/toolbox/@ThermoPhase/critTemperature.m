function v = critTemperature(tp)
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     object that derives from ThermoPhase)
% :return:
%     Critical temperature. Units: K
%

v = thermo_get(tp.tp_id,19);
