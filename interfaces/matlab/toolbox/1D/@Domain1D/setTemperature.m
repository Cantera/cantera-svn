function d = setTemperature(d, t)
% Set the temperature.
%
% :param d:
%     Instance of class :mat:func:`Domain1D`
% :param t:
%     Temperature to be set. Units: K
%

domain_methods(d.dom_id, 61, t);
