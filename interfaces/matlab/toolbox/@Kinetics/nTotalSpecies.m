function nsp = nTotalSpecies(a)
% The total number of species, summed over all
% participating phases.
%
% :param a:
%     Instance of class :mat:func:`Kinetics` (or another
%     object containing an instance of that class)
%     for which the number of species is desired.
% :return:
%     Integer total number of species
%

nsp = kinetics_get(a.id, 3, 0);
