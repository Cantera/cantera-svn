function setTolerances(r, rtol, atol)
% Set error tolerances for the solution.
%
% :param rtol:
%     Scalar relative error tolerance
% :param atol:
%     Scalar absolute error tolerance
%

reactornetmethods(7, reactornet_hndl(r), rtol, atol);
