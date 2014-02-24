function setThermalConductivity(tr, lam)
% Set the thermal conductivity.
%
% This method can only be used with transport models that
% support directly setting the value of the thermal
% conductivity.
%
% :param tr:
%     Instance of class :mat:func:`Transport`
% :param lam:
%     Thermal conductivity in W/(m-K)
%

setParameters(tr, 1, 0, lam);
