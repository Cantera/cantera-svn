function t = temperature(r)
% The temperature of the reactor in K.
%
% :param r:
%     Instance of class :mat:func:`Reactor`
% :return:
%     The temperature of the reactor contents at the
%     end of the last call to :mat:func:`advance` or :mat:func:`step`.
%

t = reactormethods(26, reactor_hndl(r));
