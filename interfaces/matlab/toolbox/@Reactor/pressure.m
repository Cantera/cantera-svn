function p = pressure(r)
% The pressure of the reactor in Pa.
%
% :param r:
%     Instance of class :mat:func:`Reactor`
% :return:
%     The pressure of the reactor contents at the
%     end of the last call to :mat:func:`advance` or :mat:func:`step`.
%

p = reactormethods(29, reactor_hndl(r));
