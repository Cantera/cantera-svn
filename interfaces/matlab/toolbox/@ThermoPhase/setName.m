function setName(tp, name)
%

if isa(name,'char')
    phase_set(thermo_hndl(tp), 32, name);
else
    error('name must be a string.');
end
