function x = atomicMasses(tp)
% Vector of element atomic masses [kg/kmol].
%
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     object that derives from ThermoPhase).
% :return:
%     Vector of element atomic masses. Units: kg/kmol
%

x = phase_get(tp.tp_id,30);
