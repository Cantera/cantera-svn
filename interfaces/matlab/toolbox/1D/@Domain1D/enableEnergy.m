function d = enableEnergy(d)
% Enable the energy equation
%
% :param d:
%     Instance of class :mat:func:`Domain1D`
%

disp(' ');
disp('Enabling the energy equation...');

domain_methods(d.dom_id, 66, 1);
