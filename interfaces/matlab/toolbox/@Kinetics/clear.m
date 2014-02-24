function clear(k)
% Delete the Kinetics instance.
%
% :param k:
%     Instance of class :mat:func:`Kinetics` (or another
%     object containing an instance of that class)
%

kinetics_set(k.id,3, 0, 0);
