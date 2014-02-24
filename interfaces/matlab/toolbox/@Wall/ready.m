function ok = ready(w)
% Checks whether a wall is ready.
%
% :param w:
%     Instance of class :mat:func:`Wall`
% :return:
%     Status of the wall
%

ok = wallmethods(11, wall_hndl(w));
