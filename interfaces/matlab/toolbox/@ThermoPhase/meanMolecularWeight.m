function wtm = meanMolecularWeight(tp)
% MEANMOLECULARWEIGHT  Get the mean molar mass.
% wtm = meanMolecularWeight(tp)
% Deprecated in favor of :mat:func:`meanMolarMass`
%
% See also: :mat:func:`meanMolarMass`
%

warning('Deprecated in favor of meanMolarMass.m')
wtm = meanMolarMass(tp);
