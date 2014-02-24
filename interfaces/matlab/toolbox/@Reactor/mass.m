function m = mass(r)
% The mass of phase in the reactor in kg.
%
% :param r:
%     Instance of class :mat:func:`Reactor`
% :return:
%     The mass of the reactor contents at the
%     end of the last call to :mat:func:`advance` or :mat:func:`step`.
%     The mass is retrieved from the solution vector.
%

m = reactormethods(23, reactor_hndl(r));
