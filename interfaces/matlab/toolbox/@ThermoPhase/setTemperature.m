function setTemperature(tp,t)
%
if (t <= 0)
    error('the temperature must be positive');
end
phase_set(tp.tp_id,1,t);
