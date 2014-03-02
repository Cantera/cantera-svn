function display(tp)
% Overload of the built-in display function to display the
% state of the ThermoPhase instance on the screen.
%
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     object that derives from ThermoPhase)
%

phase_get(thermo_hndl(tp), 15, 1);
