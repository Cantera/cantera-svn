function n = nReactions(a)
% Number of reactions.
%
% :param a:
%     Instance of class :mat:func:`Kinetics` (or another
%     object containing an instance of that class)
%     for which the number of reactions is desired.
% :return:
%     Integer number of reactions
%

n = kinetics_get(a.id,1,0);
