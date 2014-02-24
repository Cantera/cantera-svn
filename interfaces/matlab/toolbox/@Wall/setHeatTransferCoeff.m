function setHeatTransferCoeff(w, u)
% Set the heat transfer coefficient.
%
% :param w:
%     Instance of class :mat:func:`Wall`
% :param u:
%     Double, heat transfer coefficient of the wall in W/(m**2-K)
%

wallmethods(7, wall_hndl(w), u);
