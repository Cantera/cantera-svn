function setThermalResistance(w, r)
% Set the thermal resistance of a wall.
%
% :param w:
%     Instance of class :mat:func:`Wall`
% :param r:
%     Double, thermal resistance. Units: K*m**2/W
%

wallmethods(6, wall_hndl(w), r)
