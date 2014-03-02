function i = thermo_hndl(tp)
% Integer used to access kernel object
%
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     object deriving from ThermoPhase)
%     for which the handle is desired.
% :return:
%     Returns the integer ID of the kinetics kernel object.
%

i = tp.tp_id;
