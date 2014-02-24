function kr = revRateConstants(a)
% Reverse reaction rate constants for all the reactions.
%
% :param a:
%     Instance of class :mat:func:`Kinetics` (or another
%     object containing an instance of that class)
%     for which forward rate constants are desired.
% :return:
%     Returns a column vector of the reverse rate constants of
%     all of the reactions.
%

kr = kinetics_get(a.id,16,0);
