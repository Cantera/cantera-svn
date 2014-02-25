function q = qdot(w, t)
% Total heat transfer [W] through the wall at time t. A positive
% value corresponds to heat flowing from the left-hand reactor to
% the right-hand one.
%
% :param w:
%     Instance of class :mat:func:`Wall`
% :param t:
%     Time at which the heat transfer should be evaluated.
% :return:
%     Total heat transfer in W
%

q = wallmethods(22, wall_hndl(w), t);
