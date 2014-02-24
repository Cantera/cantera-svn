function v = electricalConductivity(a)
% Get the electrical conductivity in S/m.
%
% :param a:
%     Instance of class :mat:func:`Transport` (or another
%     object containing an instance of that class)
%     for which the electrical conductivity is desired.
% :return:
%     Electrical conductivity in S/m
%

v = trans_get(a.id, 3);
if v == -1.0
    error(geterr);
elseif v < 0.0
    error('exception raised');
end
