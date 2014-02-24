function addReactor(net, reactor)
% Add a reactor to a network
%
% :param net:
%     Instance of class :mat:func:`ReactorNet`
% :param reactor:
%     Instance of class :mat:func:`Reactor`
%

reactornetmethods(4, reactornet_hndl(net), reactor_hndl(reactor));
