function setPressure(tp,p)
%
%

if p <= 0.0
    error('the pressure must be positive')
end

thermo_set(tp.tp_id,1,p);
