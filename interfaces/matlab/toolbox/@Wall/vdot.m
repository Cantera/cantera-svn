function v = vdot(w, t)
% The rate of volumetric change [m**3/s] associated with the wall at
% time t. A positive value corresponds to the left-hand reactor volume
% increasing, and the right-hand reactor volume decreasing.
%
% :param w:
%     Instance of class :mat:func:`Wall`
% :param t:
%     Time at which the volumetric change should be calculated.
% :return:
%     Rate of volumetric change in m**3/s
%

v = wallmethods(21, wall_hndl(w), t);
