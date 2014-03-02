function setState_UV(tp,uv)
% Set the specific internal energy [J/kg] and specific volume [m**3/kg].
%
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     class derived from ThermoPhase)
% :param uv:
%     Vector of length 2 containing
%     the desired values for the specific internal energy (J/kg) and
%     specific volume (m**3/kg).
%

if uv(2) <= 0.0
    error('the specific volume must be positive');
end
thermo_set(tp.tp_id,21,uv);
