function t = temperature(d)
% Get the boundary temperature [K].
%
% :param d:
%     Instance of class :mat:func:`Domain1D`
% :return:
%     Temperature. Units: K
%

t = domain_methods(d.dom_id, 15);
