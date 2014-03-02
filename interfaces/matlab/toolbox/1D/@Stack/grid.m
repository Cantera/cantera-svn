function z = grid(s, name)
% The grid in one domain.
%
% :param s:
%     Instance of class :mat:func:`Stack`
% :param name:
%     Name of the domain for which the grid
%     should be retrieved.
% :return:
%     The grid in domain name
%

n = domainIndex(s,name);
d = s.domains(n);
z = gridPoints(d);
