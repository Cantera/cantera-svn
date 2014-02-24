function setEnergy(f, flag)
% Enable or disable solving the energy equation. If the
% energy equation is disabled, then the reactor temperature is
% constant. The parameter should be the string ``'on'`` to enable the
% energy equation, or ``'off'`` to disable it.
%
% By default, Reactor objects are created with the energy equation
% enabled, so usually this method is only needed to disable the
% energy equation for isothermal simulations. ::
%
%     >> setEnergy(r, 'on');
%     >> setEnergy(r, 'off');
%
% :param f:
%     Instance of class :mat:func:`Reactor`
% :param flag:
%     String, either ``'on'`` or ``'off`` to enable and disable
%     solving the energy equation, respectively
%

iflag = -1;
if strcmp(flag,{'on'})
    iflag = 1;
elseif strcmp(flag,{'off'})
    iflag = 0;
end
if iflag >= 0
    reactormethods(9, f.index, iflag)
else
    error('input to setEnergy not understood');
end
