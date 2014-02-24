function setKineticsMgr(r, k)
% Set the kinetics manager for an instance of :mat:func:`Reactor`.
% This method is used internally during Reactor initialization, but
% is usually not called by users.
%
% :param r:
%     Instance of class :mat:func:`Reactor`
% :param k:
%     Instance of class :mat:func:`Kinetics`, or another object
%     containing an instance of that class.
%

if ~isa(k,'Kinetics')
    error('wrong object type');
end

reactormethods(7, reactor_hndl(r), kinetics_hndl(k));
