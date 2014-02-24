function t = atol(r)
% Get the absolute error tolerance.
%
% :param r:
%     Instance of class :mat:func:`ReactorNet`
% :return:
%     Absolute error tolerance.
%

t = reactornetmethods(24, reactornet_hndl(r));

