function d = disableEnergy(d)
% Disable the energy equation
%
% :param d:
%     Instance of class :mat:func:`Domain1D`
%

disp(' ');
disp('Disabling the energy equation...');

domain_methods(d.dom_id, 66, 0);
