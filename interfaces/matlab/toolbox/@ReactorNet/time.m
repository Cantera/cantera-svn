function t = time(r)
% Get the current value of the time of the ReactorNet.
%
% :param r:
%     Instance of class :mat:func:`ReactorNet`
% :return:
%     Current time in the input ReactorNet
%

t = reactornetmethods(22, reactornet_hndl(r));
