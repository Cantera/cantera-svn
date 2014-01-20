function m = MassFlowController(upstream, downstream)
% Create a mass flow controller connecting two reactors / reservoirs.
%
% Creates an instance of class :mat:func:`FlowDevice` configured to
% simulate a mass flow controller that maintains a constant mass flow
% rate independent of upstream or downstream conditions. If two reactor
% objects are supplied as arguments, the controller is installed
% between the two reactors. Otherwise, the :mat:func:`install` method
% should be used to install the :mat:func:`MassFlowController` between
% reactors.
%
% see also: :mat:func:`FlowDevice`
%
% :param upstream:
%     Upstream :mat:func:`Reactor` or :mat:func:`Reservoir`
% :param downstream:
%     Downstream :mat:func:`Reactor` or :mat:func:`Reservoir`
% :return:
%     Instance of class :mat:func:`FlowDevice`
%

m = FlowDevice(1);
if nargin == 2
    install(m, upstream, downstream)
end
