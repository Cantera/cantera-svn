function setDensity(tp,rho)
%

if rho <= 0.0
    error('the density must be positive');
end

phase_set(tp.tp_id,2,rho);
