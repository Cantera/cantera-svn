function v = volume(r)
% The volume of the reactor in m**3.
%
% :param r:
%     Instance of class :mat:func:`Reactor`
% :return:
%     The volume of the reactor contents at the
%     end of the last call to :mat:func:`advance` or :mat:func:`step`.
%

v = reactormethods(24, reactor_hndl(r));
