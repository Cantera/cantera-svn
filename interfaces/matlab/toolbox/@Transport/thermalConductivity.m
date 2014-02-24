function v = thermalConductivity(a)
% Get the thermal conductivity in W/(m-K).
%
% :param a:
%     Instance of class :mat:func:`Transport` (or another
%     object containing an instance of that class)
%     for which the thermal conductivity is desired.
% :return:
%     Thermal conductivity in W/(m-K).
%

v = trans_get(a.id, 2);
if v == -1.0
    error(geterr);
elseif v < 0.0
    error('exception raised');
end
