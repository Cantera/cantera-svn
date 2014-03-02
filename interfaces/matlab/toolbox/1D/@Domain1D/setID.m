function d = setID(d, id)
% Set the ID tag for the domain.
%
% :param d:
%     Instance of class :mat:func:`Domain1D`
% :param id:
%     String ID to assign
%

domain_methods(d.dom_id, 54, id);
