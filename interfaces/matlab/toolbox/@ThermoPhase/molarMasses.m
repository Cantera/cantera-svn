function x = molarMasses(tp)
% MOLARMASSES  Get the molar masses of the species.
% x = molarMasses(tp)
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     object that derives from ThermoPhase)
% :return:
%     Vector of species molar masses. Units: kg/kmol
%
x = phase_get(tp.tp_id,22);
