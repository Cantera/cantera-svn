function i = domainType(d)
% Type of domain.
%
% :param d:
%     Instance of class :mat:func:`Domain1D`
% :return:
%     This function returns an integer flag denoting the domain
%     type.
%

i = domain_methods(d.dom_id, 12);

