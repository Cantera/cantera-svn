function v = satTemperature(tp, p)
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     object that derives from ThermoPhase)
% :param p:
%     Pressure in Pa
% :return:
%     Saturation temperature for pressure p. Units: K
%

v = thermo_get(tp.tp_id,23,p);
