function setState_SV(tp,sv)
%
%

if sv(1) <= 0.0
    error('the specific entropy must be positive');
end
if sv(2) <= 0.0
    error('the specific volume must be positive');
end
thermo_set(tp.tp_id,22,sv);
