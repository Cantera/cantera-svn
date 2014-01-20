function mdot = massFlowRate(f, time)
% MASSFLOWRATE - mass flow rate in kg/s
%
%   mdot = massFlowRate(f, time)
%         
%      Return the mass flow rate of the flow device ``f`` at time
%      ``time``.
%
mdot = flowdevicemethods(21, f.index, time);
