function wtm = meanMolarMass(tp)
% MEANMOLARMASS  Get the mean molar mass.
% wtm = meanMolarMass(tp)
% The mean molar mass is the mole-fraction-weighted sum of the
% molar masses of the individual species in the phase.
%
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     object that derives from ThermoPhase)
% :return:
%     Scalar double mean molar mass. Units: kg/kmol
%

wtm = phase_get(tp.tp_id,4);
