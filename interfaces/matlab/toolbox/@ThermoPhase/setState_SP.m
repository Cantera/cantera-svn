function setState_SP(tp,sp)
%
%

if sp(1) <= 0.0
    error('the specific entropy must be positive');
end
if sp(2) <= 0.0
    error('the pressure must be positive');
end
thermo_set(tp.tp_id,23,sp);
