function clear(tp)
% Delete the kernel object.
%
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     object that derives from ThermoPhase)
%

thermo_set(tp.tp_id,0,10);

