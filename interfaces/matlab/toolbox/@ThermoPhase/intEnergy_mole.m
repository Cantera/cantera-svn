function v = intEnergy_mole(a)
% INTENERGY_MOLE  Get the mole specific internal energy.
% v = intEnergy_mole(a)
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     object that derives from ThermoPhase)
% :return:
%     Vector of mole specific internal energies of the species.
%     Units: J/kmol
%

v = thermo_get(a.tp_id,3);
