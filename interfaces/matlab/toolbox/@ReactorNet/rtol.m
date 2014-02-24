function t = rtol(r)
% Get the relative error tolerance.
%
% :param r:
%     Instance of class :mat:func:`ReactorNet`
% :return:
%     Relative error tolerance.
%

t = reactornetmethods(23, reactornet_hndl(r));
