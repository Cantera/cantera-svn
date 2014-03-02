function i = hndl(p)
% Integer used to access kernel objects. Deprecated in favor of
% :mat:func:`thermo_hndl`.
%

warning('This function is deprecated in favor of thermo_hndl.m')
i = p.tp_id;
