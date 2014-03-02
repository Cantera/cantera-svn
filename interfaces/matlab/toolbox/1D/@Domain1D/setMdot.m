function d = setMdot(d, mdot)
% Set the mass flow rate
%
% :param d:
%     Instance of class :mat:func:`Domain1D`
% :param mdot:
%     Mass flow rate
%

domain_methods(d.dom_id, 60, mdot);
