function setMassFlowRate(f, mdot)
% Set the mass flow rate through a mass flow controller to a constant
% value
%
% See also: :mat:func:`MassFlowController`
%
% :param f:
%     Instance of class :mat:func:`MassFlowController`
% :param mdot:
%     Mass flow rate
%
if f.type == 1
    k = flowdevicemethods(3, f.index, mdot);
    if k < 0
        error(geterr);
    end
else
    error('Mass flow rate can only be set for mass flow controllers')
end
