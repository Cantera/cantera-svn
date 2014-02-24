function setInitialTime(r, t)
% Set the initial time at which to begin the time
% integration.
%
% If the time integration has already begun, this restarts the
% integrator using the current solution as the initial condition,
% setting the time to ``t``.
%
% :param r:
%     Instance of class :mat:func:`ReactorNet`
% :param t:
%     Time at which integration should be restarted, using the
%     current state as the initial condition.
%

reactornetmethods(5, reactornet_hndl(r), t);
