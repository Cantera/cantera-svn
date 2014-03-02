function setState_SV(tp,sv)
% Set the specific entropy [J/kg-K] and specific volume [m**3/kg].
%
% :param tp:
%     Instance of class :mat:func:`ThermoPhase` (or another
%     class derived from ThermoPhase)
% :param sv:
%     Vector of length 2 containing the desired values for the specific
%     entropy (J/kg-K) and specific volume (m**3/kg).
%

if sv(1) <= 0.0
    error('the specific entropy must be positive');
end
if sv(2) <= 0.0
    error('the specific volume must be positive');
end
thermo_set(tp.tp_id,22,sv);
