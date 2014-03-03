function setState_UV(tp,uv)
%
%

if uv(2) <= 0.0
    error('the specific volume must be positive');
end
thermo_set(tp.tp_id,21,uv);
