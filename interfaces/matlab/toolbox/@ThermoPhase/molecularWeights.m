function x = molecularWeights(a)
% MOLECULARWEIGHTS  Get the molar masses of all the species.
% x = molecularWeights(a)
% Deprecated in favor of :mat:func:`molarMasses`
%
% See also: :mat:func:`molarMasses`
%

warning('Deprecated in favor of molarMasses.m')
x = molarMasses(a);
