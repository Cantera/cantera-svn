function setInitialVolume(r, t0)
% Set the initial reactor volume
%
% :param r:
%     Instance of class :mat:func:`Reactor`
% :param v0:
%     Initial volume. Units: m**3
%

reactormethods(4, reactor_hndl(r), t0);
