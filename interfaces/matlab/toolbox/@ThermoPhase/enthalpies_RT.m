function v = enthalpies_RT(tp)
% Pure species non-dimensional enthalpies.
%
% sets array h_rt to the array of standard-state species enthalpies
% for phase 'phase' divided by RT, where R is the universal gas
% constant and T is the temperature. For gaseous species, these
% values are ideal gas enthalpies.
%
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     object that derives from ThermoPhase)
% :return:
%     Vector of sensible species enthalpies
%     divided by RT, where R is the universal gas
%     constant and T is the temperature. For gaseous species, these
%     values are ideal gas enthalpies.
%

v = thermo_get(tp.tp_id,32);
