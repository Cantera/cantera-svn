function i = hndl(x)
% Integer used to access kernel objects. Deprecated in favor of
% :mat:func:`xml_hndl`.
%

warning('This function is deprecated in favor of xml_hndl.m')
i = x.id;
