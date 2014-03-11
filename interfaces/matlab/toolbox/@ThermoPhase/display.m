function display(tp)
% DISPLAY  Display the phase on the terminal.
% display(tp)
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     object that derives from ThermoPhase)
%

phase_get(thermo_hndl(tp), 15, 1);
