function a= setState_HP(tp, hp)
% SETSTATE_HP - Set the specific enthalpy [J/kg] and pressure [Pa].
%
%    setState_HP(a, hp) sets the specific enthalpy and pressure
%    of object a, holding its composition fixed. Argument 'hp' must
%    be a vector of length 2 containing the desired values for the specific
%    enthalpy (J/kg) and pressure (Pa).
%

if hp(2) <= 0.0
    error('The pressure must be positive.');
end

thermo_set(tp.tp_id, 20, hp);
